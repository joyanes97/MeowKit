/**
 * @file app_06.cpp
 * @author Mingo
 * @brief App06 — AirMouse via BLE HID (bluego-esp32 algorithm port)
 * @version 2.0
 * @date 2026-05-12
 */
#include "air_mouse.h"
#include <cmath>

/* ── UI palette (RGB565) ── */
static constexpr uint16_t BG       = 0x1A22;
static constexpr uint16_t COL_TI   = 0xC7E6;
static constexpr uint16_t COL_OK   = 0xC7E6;
static constexpr uint16_t COL_BAD  = 0xF800;
static constexpr uint16_t COL_TXT  = 0xFFFF;
static constexpr uint16_t COL_DIM  = 0x4A86;
static constexpr uint16_t COL_FILL = 0x32C4;
static constexpr uint16_t COL_FILL_DARK = 0x2A83;

static constexpr int HDR_Y = 24;
static constexpr int FTR_Y = 214;
static constexpr int SCROLL_X = 174;
static constexpr int SCROLL_Y = 44;
static constexpr int SCROLL_W = 142;
static constexpr int SCROLL_H = 146;
static constexpr int GLOBE_CX = 88;
static constexpr int GLOBE_CY = 109;
static constexpr int GLOBE_R = 54;
static constexpr int BUB_R = 6;
static constexpr float BUB_SENS = 42.0f;

static void _drawBtBadge(LGFX_Class& lcd, bool connected)
{
    int bx = 196, by = 3, bw = 118, bh = 18;
    uint16_t fill = connected ? COL_TI : COL_DIM;
    lcd.fillRoundRect(bx, by, bw, bh, 3, fill);
    lcd.setFont(&fonts::efontCN_16);
    lcd.setTextColor(BG, fill);
    lcd.fillRect(bx + 1, by + 1, bw - 2, bh - 2, fill);
    int tw = (int)strlen(connected ? "CONNECTED" : "WAITING") * 8;
    lcd.setCursor(bx + (bw - tw) / 2, by + 2);
    lcd.print(connected ? "CONNECTED" : "WAITING");
}

static void _drawGlobeStatic(LGFX_Class& lcd)
{
    lcd.drawCircle(GLOBE_CX, GLOBE_CY, GLOBE_R, COL_TI);
    lcd.drawCircle(GLOBE_CX, GLOBE_CY, GLOBE_R - 1, COL_TI);

    lcd.drawEllipse(GLOBE_CX, GLOBE_CY, GLOBE_R, 19, COL_TI);
    lcd.drawEllipse(GLOBE_CX, GLOBE_CY, GLOBE_R, 42, COL_TI);
    lcd.drawEllipse(GLOBE_CX, GLOBE_CY, 18, GLOBE_R, COL_TI);
    lcd.drawEllipse(GLOBE_CX, GLOBE_CY, 38, GLOBE_R, COL_TI);
    lcd.drawEllipse(GLOBE_CX, GLOBE_CY, 50, 10, COL_TI);

    lcd.drawFastHLine(GLOBE_CX - GLOBE_R - 8, GLOBE_CY, 16, COL_TI);
    lcd.drawFastHLine(GLOBE_CX + GLOBE_R - 8, GLOBE_CY, 16, COL_TI);
    lcd.drawFastVLine(GLOBE_CX, GLOBE_CY - GLOBE_R - 8, 16, COL_TI);
    lcd.drawFastVLine(GLOBE_CX, GLOBE_CY + GLOBE_R - 8, 16, COL_TI);

    lcd.drawFastHLine(GLOBE_CX - 18, GLOBE_CY, 36, COL_TI);
    lcd.drawFastVLine(GLOBE_CX, GLOBE_CY - 18, 36, COL_TI);
    lcd.fillCircle(GLOBE_CX, GLOBE_CY, 3, COL_TXT);
}

static void _drawScrollPanel(LGFX_Class& lcd, bool active)
{
    uint16_t fill = active ? COL_FILL : COL_FILL_DARK;
    lcd.drawRoundRect(SCROLL_X, SCROLL_Y, SCROLL_W, SCROLL_H, 4, COL_TI);
    lcd.fillRect(SCROLL_X + 5, SCROLL_Y + 22, SCROLL_W - 10, SCROLL_H - 28, fill);

    lcd.fillRect(SCROLL_X + 4, SCROLL_Y + 4, 16, 16, COL_TI);
    lcd.setFont(&fonts::efontCN_16);
    lcd.setTextColor(BG, COL_TI);
    lcd.setCursor(SCROLL_X + 8, SCROLL_Y + 4);
    lcd.print("W");

    lcd.setTextColor(COL_TI, BG);
    lcd.setCursor(SCROLL_X + 26, SCROLL_Y + 4);
    lcd.print("Touch Scroll");

    for (int y = SCROLL_Y + 22; y < SCROLL_Y + SCROLL_H - 6; y += 4) {
        lcd.drawFastHLine(SCROLL_X + 8, y, SCROLL_W - 16, active ? COL_FILL_DARK : COL_DIM);
    }
}

namespace MOONCAKE::APPS
{

App06::App06(DEVICES* device)
    : _device(device)
{
    setAppInfo().name = "Air Mouse";
}

/* ───────────────────────────── onOpen ───────────────────────────── */
void App06::onOpen()
{
    auto& lcd = _device->Lcd;
    lcd.fillScreen(BG);

    if (!_device->imu.isEnabled()) {
        lcd.setFont(&fonts::FreeSansBold12pt7b);
        lcd.setTextColor(COL_BAD, BG);
        lcd.setTextDatum(textdatum_t::middle_center);
        lcd.drawString("IMU not found", 160, 120);
        return;
    }

    /* bluego 在 MPU6500 用 1000 dps + 42 Hz 硬件低通 + 100 Hz 采样率。
       我们 BMI270 没有等价的 DLPF API，setGyroRange(2)=500 dps 已足够覆盖
       人手摆动（< 500 °/s），分辨率最高。 */
    _device->imu.setGyroRange(2);

    _bias_x = _bias_y = _bias_z = 0;
    _residual_dx = _residual_dy = 0;
    _calibrated = false;
    _ui_was_connected = false;
    _ignore_pointer_until_ms = 0;
    _touch_was_down = false;
    _click_left = _click_right = 0;
    _cursor_x = 0;
    _cursor_y = 0;
    _ui_prev_bx = GLOBE_CX;
    _ui_prev_by = GLOBE_CY;
    _b_longpress_latched = false;
    _touch_scroll_active = false;
    _touch_last_y = -1;
    _last_us = micros();

    _drawStaticUI();

    _mouse.begin();
    Serial.println("[App06] AirMouse: BLE advertising as 'MeowKit AirMouse'");

    _calibrate();
    _drawStaticUI();
    _updateUI(0.0f, 0.0f, 0.0f, 0.0f, 0, 0, false);
}

/* ───────────────────────────── onRunning ───────────────────────────── */
/* 完全照搬 bluego/imu_gyro_task 的核心：dt 积分 → 0.02° 死区 → 1°=50px 映射。 */
void App06::onRunning()
{
    if (!_device->imu.isEnabled()) return;

    _device->button.update();
    _device->button.tick();

    if (_device->button.B.pressed()) {
        _b_longpress_latched = false;
    }
    if (_device->button.B.isLongPress()) {
        _b_longpress_latched = true;
        close();
        return;
    }

    auto mask = _device->imu.update();
    if (!(mask & IMU_Class::sensor_mask_gyro)) return;
    auto& d = _device->imu.getImuData();

    uint32_t now_us = micros();
    float dt = (now_us - _last_us) * 1e-6f;
    _last_us = now_us;
    if (dt <= 0) dt = 0.005f;
    if (dt > DT_CLAMP_SEC) dt = DT_CLAMP_SEC;   // bluego: cap 100ms

    uint32_t now_ms = millis();

    /* 原始角速度 (°/s) */
    float raw_wx = d.gyro.x;
    float raw_wy = d.gyro.y;
    float raw_wz = d.gyro.z;

    /* 漂移修正：静止时缓慢收敛零偏。bluego 没做这个，但 BMI270 漂移更明显，
       保留软件零偏跟随既不破坏 bluego 的线性手感，又能消除长时间漂移。 */
    float acc_norm  = sqrtf(d.accel.x*d.accel.x + d.accel.y*d.accel.y + d.accel.z*d.accel.z);
    float gyro_norm = sqrtf(raw_wx*raw_wx + raw_wy*raw_wy + raw_wz*raw_wz);
    bool stationary = (fabsf(acc_norm - 1.0f) < STATIONARY_ACC_TOL)
                   && (gyro_norm < STATIONARY_GYRO_DPS);
    if (stationary) {
        _bias_x += BIAS_ALPHA * (raw_wx - _bias_x);
        _bias_y += BIAS_ALPHA * (raw_wy - _bias_y);
        _bias_z += BIAS_ALPHA * (raw_wz - _bias_z);
    }

    float wy = raw_wy - _bias_y;   // pitch (绕水平 Y 轴)  → ΔY
    float wz = raw_wz - _bias_z;   // yaw   (绕垂直 Z 轴)  → ΔX

    /* 本周期角度增量 (°) — bluego 的 angle_diff */
    float ang_y = wy * dt;
    float ang_z = wz * dt;

    /* bluego 触发条件： |angle_diff| * 100 >= 2  →  |angle_diff| >= 0.02° */
    bool trig = (fabsf(ang_y) >= DEAD_DEG) || (fabsf(ang_z) >= DEAD_DEG);

    int dx = 0, dy = 0;
    if (trig) {
        /* 1° = 1 / 0.02 = 50 像素，与 bluego 一致 */
        float fx = ang_z / DEG_PER_PIXEL;   // gyro.z → ΔX
        float fy = ang_y / DEG_PER_PIXEL;   // gyro.y → ΔY
        if (INV_X) fx = -fx;
        if (INV_Y) fy = -fy;

        /* 残差累加，避免亚像素丢失 */
        _residual_dx += fx;
        _residual_dy += fy;

        dx = (int)_residual_dx;  _residual_dx -= dx;
        dy = (int)_residual_dy;  _residual_dy -= dy;

        if (dx >  127) dx =  127; else if (dx < -127) dx = -127;
        if (dy >  127) dy =  127; else if (dy < -127) dy = -127;
    } else {
        /* 衰减残差，防止抖动期间累积造成 idle 后突跳 */
        _residual_dx *= 0.5f;
        _residual_dy *= 0.5f;
    }

    /* bluego 风格的"点击稳定"门控：
       若刚触发了左/右键，IGNORE_POINTER_MS 内不再上报指针位移。
       目前 App06 还没有点击源（之前的扭动点击已按 bluego 风格移除），
       此处保留接口，等接入物理按钮后调用 _ignore_pointer_until_ms = now+200。 */
    bool ignore_pointer = (int32_t)(now_ms - _ignore_pointer_until_ms) < 0;

    if (_mouse.isConnected() && (dx || dy) && !ignore_pointer) {
        _mouse.move((signed char)dx, (signed char)dy, 0, 0);
        _cursor_x += dx;
        _cursor_y += dy;
    }

    if (_device->button.A.pressed() && _mouse.isConnected()) {
        _mouse.click(MOUSE_LEFT);
        _ignore_pointer_until_ms = now_ms + IGNORE_POINTER_MS;
        ++_click_left;
    }
    if (_device->button.B.released() && !_b_longpress_latched && _mouse.isConnected()) {
        _mouse.click(MOUSE_RIGHT);
        _ignore_pointer_until_ms = now_ms + IGNORE_POINTER_MS;
        ++_click_right;
    }
    if (_device->button.B.released()) {
        _b_longpress_latched = false;
    }

    _handleTouch(now_ms);

    if (now_ms - _last_ui_ms > UI_PERIOD_MS) {
        _last_ui_ms = now_ms;
        _updateUI(d.accel.x, d.accel.y, wy, wz, dx, dy, dx || dy);
    }
}

/* ───────────────────────────── onClose ───────────────────────────── */
void App06::onClose()
{
    _mouse.end();
    _device->Lcd.fillScreen(TFT_BLACK);
    Serial.println("[App06] AirMouse closed");
}

/* ───────────────────────────── Touch → Mouse wheel ─────────────────────────────
 * 右侧触摸区垂直滑动映射为滚轮。
 */
void App06::_handleTouch(uint32_t now_ms)
{
    bool down = _device->ctp.isTouched();
    bool conn = _mouse.isConnected();

    if (down && !_touch_was_down) {
        int tx = -1, ty = -1;
        _device->ctp.getPos(tx, ty);
        bool inScroll = (tx >= SCROLL_X && tx < SCROLL_X + SCROLL_W &&
                         ty >= SCROLL_Y && ty < SCROLL_Y + SCROLL_H);
        if (inScroll) {
            _touch_scroll_active = true;
            _touch_last_y = ty;
            _ignore_pointer_until_ms = now_ms + IGNORE_POINTER_MS;
        }
    } else if (down && _touch_scroll_active) {
        int tx = -1, ty = -1;
        _device->ctp.getPos(tx, ty);
        if (_touch_last_y >= 0) {
            int delta = ty - _touch_last_y;
            if (abs(delta) >= 10 && conn) {
                int wheel = -delta / 10;
                if (wheel > 4) wheel = 4;
                if (wheel < -4) wheel = -4;
                _mouse.move(0, 0, (signed char)wheel, 0);
                _touch_last_y = ty;
            }
        }
    } else if (!down) {
        _touch_scroll_active = false;
        _touch_last_y = -1;
    }
    _touch_was_down = down;
}

/* ───────────────────────────── Calibration ───────────────────────────── */
void App06::_calibrate()
{
    auto& lcd = _device->Lcd;
    lcd.setFont(&fonts::efontCN_16);
    lcd.setTextDatum(textdatum_t::middle_center);
    lcd.fillRect(68, 194, 184, 16, BG);
    lcd.setTextColor(COL_TI, BG);
    lcd.drawString("Calibrating gyro...", 160, 202);

    double sx = 0, sy = 0, sz = 0;
    uint32_t got = 0;
    uint32_t t0 = millis();
    while (got < CAL_SAMPLES && (millis() - t0) < CAL_TIMEOUT_MS) {
        auto m = _device->imu.update();
        if (m & IMU_Class::sensor_mask_gyro) {
            auto& d = _device->imu.getImuData();
            sx += d.gyro.x;  sy += d.gyro.y;  sz += d.gyro.z;
            ++got;
        }
        delay(2);
    }
    if (got > 10) {
        _bias_x = (float)(sx / got);
        _bias_y = (float)(sy / got);
        _bias_z = (float)(sz / got);
        _calibrated = true;
        Serial.printf("[App06] Gyro bias: x=%.3f y=%.3f z=%.3f (n=%lu)\n",
                      _bias_x, _bias_y, _bias_z, (unsigned long)got);
    } else {
        Serial.println("[App06] Calibration failed (no samples)");
    }

    lcd.fillRect(68, 194, 184, 16, BG);
    _last_us = micros();
}

/* ───────────────────────────── UI ───────────────────────────── */
void App06::_drawStaticUI()
{
    auto& lcd = _device->Lcd;
    lcd.fillScreen(BG);

    lcd.setFont(&fonts::efontCN_16);
    lcd.setTextDatum(textdatum_t::top_left);
    lcd.setTextColor(COL_TXT, BG);
    lcd.setCursor(8, 4);
    lcd.print("[ AIR MOUSE ]");
    lcd.drawFastHLine(0, HDR_Y, 320, COL_TI);
    lcd.drawFastHLine(0, FTR_Y, 320, COL_TI);
    lcd.drawFastHLine(0, 239, 320, COL_TI);

    _drawBtBadge(lcd, _mouse.isConnected());
    _drawGlobeStatic(lcd);
    _drawScrollPanel(lcd, false);

    lcd.setTextColor(COL_TI, BG);
    lcd.setCursor(GLOBE_CX - 20, 176);
    lcd.print("LEVEL");

    _ui_was_connected = !_mouse.isConnected();
    _ui_prev_bx = GLOBE_CX;
    _ui_prev_by = GLOBE_CY;
}

void App06::_updateUI(float ax_g, float ay_g, float wy_dps, float wz_dps,
                      int dx, int dy, bool moving)
{
    auto& lcd = _device->Lcd;
    lcd.setFont(&fonts::efontCN_16);
    lcd.setTextDatum(textdatum_t::top_left);
    char buf[48];

    bool conn = _mouse.isConnected();
    if (conn != _ui_was_connected) {
        _drawBtBadge(lcd, conn);
        _ui_was_connected = conn;
    }

    _drawScrollPanel(lcd, _touch_scroll_active);

    int bx = GLOBE_CX + (int)(ay_g * BUB_SENS);
    int by = GLOBE_CY + (int)(-ax_g * BUB_SENS);
    float offx = (float)(bx - GLOBE_CX);
    float offy = (float)(by - GLOBE_CY);
    float dist = sqrtf(offx * offx + offy * offy);
    float maxD = (float)(GLOBE_R - BUB_R - 2);
    if (dist > maxD && dist > 0.1f) {
        float s = maxD / dist;
        bx = GLOBE_CX + (int)(offx * s);
        by = GLOBE_CY + (int)(offy * s);
    }

    if (abs(bx - _ui_prev_bx) >= 1 || abs(by - _ui_prev_by) >= 1) {
        lcd.startWrite();
        lcd.fillCircle(_ui_prev_bx, _ui_prev_by, BUB_R + 2, BG);
        _drawGlobeStatic(lcd);
        lcd.fillCircle(bx, by, BUB_R, COL_TI);
        lcd.fillCircle(bx, by, BUB_R - 3, COL_TXT);
        lcd.endWrite();
        _ui_prev_bx = bx;
        _ui_prev_by = by;
    }

    lcd.fillRect(8, 194, 150, 16, BG);
    lcd.setTextColor(COL_TI, BG);
    lcd.setCursor(8, 194);
    snprintf(buf, sizeof(buf), "X:%4d Y:%4d", _cursor_x, _cursor_y);
    lcd.print(buf);

    lcd.fillRect(8, 176, 80, 16, BG);
    lcd.setTextColor(moving ? COL_TI : COL_DIM, BG);
    lcd.setCursor(8, 176);
    snprintf(buf, sizeof(buf), "%s", moving ? "MOVING" : "LEVEL");
    lcd.print(buf);

    lcd.fillRect(0, 216, 320, 22, BG);
    lcd.setTextColor(COL_TI, BG);
    lcd.setCursor(8, 220);
    lcd.print("Touch Scroll");
    lcd.setCursor(118, 220);
    lcd.print("[A] Left");
    lcd.setCursor(226, 220);
    lcd.print("[B] Right");

    (void)wy_dps;
    (void)wz_dps;
    (void)dx;
    (void)dy;
}

}  // namespace MOONCAKE::APPS

