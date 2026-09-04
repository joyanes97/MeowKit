/**
 * @file app_06.h
 * @author Mingo
 * @brief App06 — AirMouse (BLE HID, ported from bluego-esp32)
 * @version 2.0
 * @date 2026-05-12
 *
 * 体感空中飞鼠 — 算法移植自 GeekFantasy/bluego-esp32：
 *   - 固定周期采样 + dt 积分得本周期角度增量 angle_diff (°)
 *   - 像素映射： point_x = -gyro.z 积分 / 0.02   (yaw  → ΔX，1° = 50 像素)
 *               point_y =  gyro.y 积分 / 0.02   (pitch → ΔY，1° = 50 像素)
 *   - 触发死区：|Δθ| > 0.02° (≈ 整像素)
 *   - 静止时自适应零偏修正
 *   - 上电静置校准
 *   - 点击稳定接口（左/右键触发后短暂忽略指针上报）
 *   - BLE HID 配对名 "MeowKit AirMouse"
 *
 */
#pragma once
#include <mooncake.h>
#include <BleMouse.h>
#include "../../bsp/devices.h"

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App06 : public AppAbility {
    public:
        App06(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;

    private:
        DEVICES* _device = nullptr;
        BleMouse  _mouse{"MeowKit AirMouse", "MeowKit", 100};

        /* ── Tunable parameters (bluego style) ── */
        // 每多少度对应 1 像素。bluego: 0.02° = 1 px
        static constexpr float DEG_PER_PIXEL = 0.02f;
        // 每周期角度增量小于该值时视为 0（与 bluego 0.02° 一致）
        static constexpr float DEAD_DEG      = 0.02f;
        // 极性反转：根据外壳安装方向（实测调整）
        static constexpr bool  INV_X         = false;
        static constexpr bool  INV_Y         = false;
        // dt clamp 上限（与 bluego 一致：100 ms）
        static constexpr float DT_CLAMP_SEC  = 0.100f;
        // 静止判定 + 零偏自适应
        static constexpr float STATIONARY_GYRO_DPS = 8.0f;
        static constexpr float STATIONARY_ACC_TOL  = 0.08f;
        static constexpr float BIAS_ALPHA          = 0.01f;
        // 上电校准
        static constexpr uint32_t CAL_SAMPLES    = 200;
        static constexpr uint32_t CAL_TIMEOUT_MS = 3000;
        // 点击稳定窗口（bluego: IGNORE_POINTER_SPAN = 200ms）
        static constexpr uint32_t IGNORE_POINTER_MS = 200;
        // UI 刷新间隔
        static constexpr uint32_t UI_PERIOD_MS = 100;

        /* ── Runtime state ── */
        float    _bias_x = 0, _bias_y = 0, _bias_z = 0;
        float    _residual_dx = 0, _residual_dy = 0;
        bool     _calibrated = false;
        uint32_t _last_us = 0;
        uint32_t _ignore_pointer_until_ms = 0;
        bool     _ui_was_connected = false;
        uint32_t _last_ui_ms = 0;
        int      _cursor_x = 0;
        int      _cursor_y = 0;
        int      _ui_prev_bx = 160;
        int      _ui_prev_by = 110;
        bool     _b_longpress_latched = false;
        bool     _touch_scroll_active = false;
        int      _touch_last_y = -1;

        /* 触摸 → 右侧滚轮区 */
        bool     _touch_was_down = false;
        unsigned long _click_left  = 0;
        unsigned long _click_right = 0;

        void _drawStaticUI();
        void _calibrate();
        void _updateUI(float ax_g, float ay_g, float wy_dps, float wz_dps,
                       int dx, int dy, bool moving);
        void _handleTouch(uint32_t now_ms);
    };
}
