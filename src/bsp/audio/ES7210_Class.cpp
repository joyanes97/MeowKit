/**
 * @file ES7210_Class.cpp
 * @brief ES7210 4-channel Audio ADC driver implementation
 * 
 * Uses I2C_Device base class for register access via I2C_Class.
 */

#include "ES7210_Class.hpp"
#include <esp_log.h>

static const char* TAG = "ES7210";

// ── Clock coefficient table ──
// Fields: mclk, lrck, ss_ds, adc_div, dll, doubler, osr, lrck_h, lrck_l
// ss_ds: 0=single speed (<=48kHz), 1=double speed (>=64kHz)
const ES7210_Class::CoeffDiv ES7210_Class::_coeff_table[] = {
    //mclk       lrck   ss adc_div dll doubler osr  lrck_h lrck_l
    /* 8k */
    {12288000,   8000,  0, 0x03, 0x01, 0x00, 0x20, 0x06, 0x00},
    {16384000,   8000,  0, 0x04, 0x01, 0x00, 0x20, 0x08, 0x00},
    {19200000,   8000,  0, 0x1e, 0x00, 0x01, 0x28, 0x09, 0x60},
    { 4096000,   8000,  0, 0x01, 0x01, 0x00, 0x20, 0x02, 0x00},
    /* 11.025k */
    {11289600,  11025,  0, 0x02, 0x01, 0x00, 0x20, 0x01, 0x00},
    /* 12k */
    {12288000,  12000,  0, 0x02, 0x01, 0x00, 0x20, 0x04, 0x00},
    {19200000,  12000,  0, 0x14, 0x00, 0x01, 0x28, 0x06, 0x40},
    /* 16k */
    { 4096000,  16000,  0, 0x01, 0x01, 0x01, 0x20, 0x01, 0x00},
    {19200000,  16000,  0, 0x0a, 0x00, 0x00, 0x1e, 0x04, 0x80},
    {16384000,  16000,  0, 0x02, 0x01, 0x00, 0x20, 0x04, 0x00},
    {12288000,  16000,  0, 0x03, 0x01, 0x01, 0x20, 0x03, 0x00},
    /* 22.05k */
    {11289600,  22050,  0, 0x01, 0x01, 0x00, 0x20, 0x02, 0x00},
    /* 24k */
    {12288000,  24000,  0, 0x01, 0x01, 0x00, 0x20, 0x02, 0x00},
    {19200000,  24000,  0, 0x0a, 0x00, 0x01, 0x28, 0x03, 0x20},
    /* 32k */
    {12288000,  32000,  0, 0x03, 0x00, 0x00, 0x20, 0x01, 0x80},
    {16384000,  32000,  0, 0x01, 0x01, 0x00, 0x20, 0x02, 0x00},
    {19200000,  32000,  0, 0x05, 0x00, 0x00, 0x1e, 0x02, 0x58},
    /* 44.1k */
    {11289600,  44100,  0, 0x01, 0x01, 0x01, 0x20, 0x01, 0x00},
    /* 48k */
    {12288000,  48000,  0, 0x01, 0x01, 0x01, 0x20, 0x01, 0x00},
    {19200000,  48000,  0, 0x05, 0x00, 0x01, 0x28, 0x01, 0x90},
    /* 64k — double speed */
    {16384000,  64000,  1, 0x01, 0x01, 0x00, 0x20, 0x01, 0x00},
    {19200000,  64000,  0, 0x05, 0x00, 0x01, 0x1e, 0x01, 0x2c},
    /* 88.2k — double speed */
    {11289600,  88200,  1, 0x01, 0x01, 0x01, 0x20, 0x00, 0x80},
    /* 96k — double speed */
    {12288000,  96000,  1, 0x01, 0x01, 0x01, 0x20, 0x00, 0x80},
    {19200000,  96000,  1, 0x05, 0x00, 0x01, 0x28, 0x00, 0xc8},
};

const size_t ES7210_Class::_coeff_table_size =
    sizeof(ES7210_Class::_coeff_table) / sizeof(ES7210_Class::_coeff_table[0]);

// ── Constructor ──
ES7210_Class::ES7210_Class(uint8_t addr, I2C_Class* i2c)
    : I2C_Device(addr, 400000, i2c)
{
}

// ── Public methods ──

bool ES7210_Class::begin(uint32_t sample_rate, es7210_bits_t bits,
                        es7210_fmt_t fmt, es7210_signal_t signal)
{
    _sample_rate = sample_rate;
    _bits   = bits;
    _fmt    = fmt;
    _signal = signal;

    if (!isPresent()) {
        ESP_LOGE(TAG, "Device not found at 0x%02X", _addr);
        return false;
    }
    ESP_LOGI(TAG, "Device found at 0x%02X", _addr);

    if (!resetChip()) {
        ESP_LOGE(TAG, "Reset failed");
        return false;
    }

    bool ok = true;
    // Basic setup
    ok &= writeRegister8(ES7210_CLOCK_OFF_REG01, 0x1f);
    ok &= writeRegister8(ES7210_TIME_CONTROL0_REG09, 0x30);
    ok &= writeRegister8(ES7210_TIME_CONTROL1_REG0A, 0x30);
    ok &= writeRegister8(ES7210_ADC12_HPF2_REG23, 0x2a);
    ok &= writeRegister8(ES7210_ADC12_HPF1_REG22, 0x0a);
    ok &= writeRegister8(ES7210_ADC34_HPF2_REG20, 0x0a);
    ok &= writeRegister8(ES7210_ADC34_HPF1_REG21, 0x2a);

    // Slave mode
    ok &= updateBits(ES7210_MODE_CONFIG_REG08, 0x01, 0x00);
    ESP_LOGI(TAG, "Slave mode");

    // Analog
    ok &= writeRegister8(ES7210_ANALOG_REG40, 0x43);
    ok &= writeRegister8(ES7210_MIC12_BIAS_REG41, 0x70);
    ok &= writeRegister8(ES7210_MIC34_BIAS_REG42, 0x70);
    ok &= writeRegister8(ES7210_OSR_REG07, 0x20);
    ok &= writeRegister8(ES7210_MAINCLK_REG02, 0xc1);

    ok &= setSampleRate(_sample_rate);
    ok &= setBits(_bits);
    ok &= setFormat(_fmt);
    ok &= selectMic(_mic_mask);
    ok &= setGain(_gain);

    // Configure REG12 for TDM or normal I2S
    uint8_t reg12 = (_signal == ES7210_SIGNAL_TDM) ? 0x02 : 0x00;
    ok &= writeRegister8(ES7210_SDP_INTERFACE2_REG12, reg12);
    ESP_LOGI(TAG, "Signal type: %s", (_signal == ES7210_SIGNAL_TDM) ? "TDM" : "I2S");

    _init = ok;
    if (ok) {
        ESP_LOGI(TAG, "Init OK (rate=%lu, bits=%d)", _sample_rate, _bits);
    } else {
        ESP_LOGE(TAG, "Init FAILED");
    }
    return ok;
}

void ES7210_Class::end()
{
    if (_started) stop();
    writeRegister8(ES7210_POWER_DOWN_REG06, 0x07);
    _init = false;
    ESP_LOGI(TAG, "De-initialized");
}

bool ES7210_Class::start()
{
    if (!_init) { ESP_LOGE(TAG, "Not initialized"); return false; }

    uint8_t regv = readRegister8(ES7210_CLOCK_OFF_REG01);
    if (regv != 0x7f && regv != 0xff) {
        _clk_reg_save = regv;
    }

    bool ok = true;
    ok &= writeRegister8(ES7210_CLOCK_OFF_REG01, _clk_reg_save);
    ok &= writeRegister8(ES7210_POWER_DOWN_REG06, 0x00);
    ok &= writeRegister8(ES7210_ANALOG_REG40, 0x43);
    ok &= writeRegister8(ES7210_MIC1_POWER_REG47, 0x00);
    ok &= writeRegister8(ES7210_MIC2_POWER_REG48, 0x00);
    ok &= writeRegister8(ES7210_MIC3_POWER_REG49, 0x00);
    ok &= writeRegister8(ES7210_MIC4_POWER_REG4A, 0x00);
    ok &= selectMic(_mic_mask);

    _started = ok;
    ESP_LOGI(TAG, "ADC %s", ok ? "started" : "start FAILED");
    return ok;
}

bool ES7210_Class::stop()
{
    if (!_init) return false;

    uint8_t regv = readRegister8(ES7210_CLOCK_OFF_REG01);
    _clk_reg_save = regv;

    bool ok = true;
    ok &= writeRegister8(ES7210_MIC1_POWER_REG47, 0xff);
    ok &= writeRegister8(ES7210_MIC2_POWER_REG48, 0xff);
    ok &= writeRegister8(ES7210_MIC3_POWER_REG49, 0xff);
    ok &= writeRegister8(ES7210_MIC4_POWER_REG4A, 0xff);
    ok &= writeRegister8(ES7210_MIC12_POWER_REG4B, 0xff);
    ok &= writeRegister8(ES7210_MIC34_POWER_REG4C, 0xff);
    ok &= writeRegister8(ES7210_ANALOG_REG40, 0xc0);
    ok &= writeRegister8(ES7210_CLOCK_OFF_REG01, 0x7f);
    ok &= writeRegister8(ES7210_POWER_DOWN_REG06, 0x07);

    _started = !ok ? _started : false;
    ESP_LOGI(TAG, "ADC %s", ok ? "stopped" : "stop FAILED");
    return ok;
}

bool ES7210_Class::setSampleRate(uint32_t rate)
{
    uint32_t mclk = rate * MCLK_DIV_FRE;
    int idx = findCoeff(mclk, rate);
    if (idx < 0) {
        ESP_LOGE(TAG, "Unsupported rate %lu (mclk=%lu)", rate, mclk);
        return false;
    }
    auto& c = _coeff_table[idx];

    bool ok = true;

    // Set adc_div & doubler & dll
    uint8_t regv = 0x00;
    regv |= c.adc_div;
    regv |= c.doubler << 6;
    regv |= c.dll << 7;
    ok &= writeRegister8(ES7210_MAINCLK_REG02, regv);

    // Set osr
    ok &= writeRegister8(ES7210_OSR_REG07, c.osr);

    // Set lrck divider
    ok &= writeRegister8(ES7210_LRCK_DIVH_REG04, c.lrck_h);
    ok &= writeRegister8(ES7210_LRCK_DIVL_REG05, c.lrck_l);

    // Set single/double speed mode via REG08 bit[6]
    // ss_ds=1 for sample rates >= 64kHz (double speed)
    ok &= updateBits(ES7210_MODE_CONFIG_REG08, 0x40, c.ss_ds ? 0x40 : 0x00);

    if (ok) _sample_rate = rate;
    ESP_LOGI(TAG, "Sample rate => %lu Hz (ss_ds=%d)", rate, c.ss_ds);
    return ok;
}

bool ES7210_Class::setBits(es7210_bits_t bits)
{
    uint8_t iface = readRegister8(ES7210_SDP_INTERFACE1_REG11);
    iface &= 0x1f;
    switch (bits) {
        case ES7210_BIT_16: iface |= 0x60; break;
        case ES7210_BIT_24: iface |= 0x00; break;
        case ES7210_BIT_32: iface |= 0x80; break;
    }
    bool ok = writeRegister8(ES7210_SDP_INTERFACE1_REG11, iface);
    if (ok) _bits = bits;
    return ok;
}

bool ES7210_Class::setFormat(es7210_fmt_t fmt)
{
    uint8_t iface = readRegister8(ES7210_SDP_INTERFACE1_REG11);
    iface &= 0xfc;
    switch (fmt) {
        case ES7210_FMT_I2S: iface |= 0x00; break;
        case ES7210_FMT_LJ:
        case ES7210_FMT_RJ:  iface |= 0x01; break;
        case ES7210_FMT_DSP: iface |= 0x03; break;
    }
    bool ok = writeRegister8(ES7210_SDP_INTERFACE1_REG11, iface);
    if (ok) _fmt = fmt;
    return ok;
}

bool ES7210_Class::selectMic(uint8_t mic_mask)
{
    if (!(mic_mask & 0x0F)) {
        ESP_LOGE(TAG, "Invalid mic mask 0x%02X", mic_mask);
        return false;
    }

    bool ok = true;
    // Disable all gain enables first
    for (int i = 0; i < 4; i++) {
        ok &= updateBits(ES7210_MIC1_GAIN_REG43 + i, 0x10, 0x00);
    }
    ok &= writeRegister8(ES7210_MIC12_POWER_REG4B, 0xff);
    ok &= writeRegister8(ES7210_MIC34_POWER_REG4C, 0xff);

    if (mic_mask & ES7210_MIC1) {
        ok &= updateBits(ES7210_CLOCK_OFF_REG01, 0x0b, 0x00);
        ok &= writeRegister8(ES7210_MIC12_POWER_REG4B, 0x00);
        ok &= updateBits(ES7210_MIC1_GAIN_REG43, 0x10, 0x10);
    }
    if (mic_mask & ES7210_MIC2) {
        ok &= updateBits(ES7210_CLOCK_OFF_REG01, 0x0b, 0x00);
        ok &= writeRegister8(ES7210_MIC12_POWER_REG4B, 0x00);
        ok &= updateBits(ES7210_MIC2_GAIN_REG44, 0x10, 0x10);
    }
    if (mic_mask & ES7210_MIC3) {
        ok &= updateBits(ES7210_CLOCK_OFF_REG01, 0x15, 0x00);
        ok &= writeRegister8(ES7210_MIC34_POWER_REG4C, 0x00);
        ok &= updateBits(ES7210_MIC3_GAIN_REG45, 0x10, 0x10);
    }
    if (mic_mask & ES7210_MIC4) {
        ok &= updateBits(ES7210_CLOCK_OFF_REG01, 0x15, 0x00);
        ok &= writeRegister8(ES7210_MIC34_POWER_REG4C, 0x00);
        ok &= updateBits(ES7210_MIC4_GAIN_REG46, 0x10, 0x10);
    }

    if (ok) _mic_mask = mic_mask;
    return ok;
}

bool ES7210_Class::setGain(es7210_gain_t gain)
{
    if (gain > ES7210_GAIN_37_5DB) gain = ES7210_GAIN_37_5DB;

    bool ok = true;
    if (_mic_mask & ES7210_MIC1) ok &= updateBits(ES7210_MIC1_GAIN_REG43, 0x0f, gain);
    if (_mic_mask & ES7210_MIC2) ok &= updateBits(ES7210_MIC2_GAIN_REG44, 0x0f, gain);
    if (_mic_mask & ES7210_MIC3) ok &= updateBits(ES7210_MIC3_GAIN_REG45, 0x0f, gain);
    if (_mic_mask & ES7210_MIC4) ok &= updateBits(ES7210_MIC4_GAIN_REG46, 0x0f, gain);

    if (ok) _gain = gain;
    return ok;
}

int ES7210_Class::getGain()
{
    uint8_t reg = 0;
    if      (_mic_mask & ES7210_MIC1) reg = ES7210_MIC1_GAIN_REG43;
    else if (_mic_mask & ES7210_MIC2) reg = ES7210_MIC2_GAIN_REG44;
    else if (_mic_mask & ES7210_MIC3) reg = ES7210_MIC3_GAIN_REG45;
    else if (_mic_mask & ES7210_MIC4) reg = ES7210_MIC4_GAIN_REG46;
    else return -1;

    return readRegister8(reg) & 0x0f;
}

bool ES7210_Class::setMute(bool enable)
{
    // Mute via ADC automute register
    return writeRegister8(ES7210_ADC_AUTOMUTE_REG13, enable ? 0xFF : 0x00);
}

bool ES7210_Class::setChannels(uint8_t ch)
{
    switch (ch) {
        case 2:
            return selectMic(ES7210_MIC1 | ES7210_MIC2);
        case 4:
            return selectMic(ES7210_MIC1 | ES7210_MIC2 | ES7210_MIC3 | ES7210_MIC4);
        default:
            ESP_LOGE(TAG, "Unsupported channel count: %d (use 2 or 4)", ch);
            return false;
    }
}

bool ES7210_Class::isPresent()
{
    return _i2c->scanID(_addr);
}

void ES7210_Class::printStatus()
{
    ESP_LOGI(TAG, "=== ES7210 Status ===");
    ESP_LOGI(TAG, "  I2C addr : 0x%02X", _addr);
    ESP_LOGI(TAG, "  Init     : %s", _init ? "Y" : "N");
    ESP_LOGI(TAG, "  Running  : %s", _started ? "Y" : "N");
    ESP_LOGI(TAG, "  Rate     : %lu Hz", _sample_rate);
    ESP_LOGI(TAG, "  Bits     : %d", _bits);
    ESP_LOGI(TAG, "  Format   : %d", _fmt);
    ESP_LOGI(TAG, "  MIC mask : 0x%02X", _mic_mask);
    ESP_LOGI(TAG, "  Gain     : %d", _gain);
}

void ES7210_Class::dumpRegisters()
{
    ESP_LOGI(TAG, "=== Register Dump ===");
    for (int i = 0; i <= 0x4E; i++) {
        uint8_t v = readRegister8(i);
        ESP_LOGI(TAG, "  REG[0x%02X] = 0x%02X", i, v);
    }
}

// ── Private methods ──

bool ES7210_Class::updateBits(uint8_t reg, uint8_t mask, uint8_t data)
{
    uint8_t old_val = readRegister8(reg);
    uint8_t new_val = (old_val & ~mask) | (mask & data);
    return writeRegister8(reg, new_val);
}

int ES7210_Class::findCoeff(uint32_t mclk, uint32_t lrck)
{
    for (size_t i = 0; i < _coeff_table_size; i++) {
        if (_coeff_table[i].lrck == lrck && _coeff_table[i].mclk == mclk) {
            return (int)i;
        }
    }
    return -1;
}

bool ES7210_Class::resetChip()
{
    bool ok = writeRegister8(ES7210_RESET_REG00, 0xff);
    vTaskDelay(pdMS_TO_TICKS(10));
    ok &= writeRegister8(ES7210_RESET_REG00, 0x41);
    vTaskDelay(pdMS_TO_TICKS(10));
    return ok;
}
