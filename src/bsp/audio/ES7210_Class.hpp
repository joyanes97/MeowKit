/**
 * @file ES7210_Class.hpp
 * @brief ES7210 4-channel Audio ADC driver using I2C_Device base class
 * 
 * Ported from Espressif ES7210 driver, adapted to MeowKit BSP I2C infrastructure.
 * Replaces TwoWire with I2C_Class/I2C_Device.
 */
#pragma once

#include "../i2c/I2C_Class.hpp"
#include <cstdint>

// ES7210 Register Definitions
#define ES7210_RESET_REG00                 0x00
#define ES7210_CLOCK_OFF_REG01             0x01
#define ES7210_MAINCLK_REG02               0x02
#define ES7210_MASTER_CLK_REG03            0x03
#define ES7210_LRCK_DIVH_REG04             0x04
#define ES7210_LRCK_DIVL_REG05             0x05
#define ES7210_POWER_DOWN_REG06            0x06
#define ES7210_OSR_REG07                   0x07
#define ES7210_MODE_CONFIG_REG08           0x08
#define ES7210_TIME_CONTROL0_REG09         0x09
#define ES7210_TIME_CONTROL1_REG0A         0x0A
#define ES7210_SDP_INTERFACE1_REG11        0x11
#define ES7210_SDP_INTERFACE2_REG12        0x12
#define ES7210_ADC_AUTOMUTE_REG13          0x13
#define ES7210_ADC34_MUTERANGE_REG14       0x14
#define ES7210_ADC34_HPF2_REG20            0x20
#define ES7210_ADC34_HPF1_REG21            0x21
#define ES7210_ADC12_HPF1_REG22            0x22
#define ES7210_ADC12_HPF2_REG23            0x23
#define ES7210_ANALOG_REG40                0x40
#define ES7210_MIC12_BIAS_REG41            0x41
#define ES7210_MIC34_BIAS_REG42            0x42
#define ES7210_MIC1_GAIN_REG43             0x43
#define ES7210_MIC2_GAIN_REG44             0x44
#define ES7210_MIC3_GAIN_REG45             0x45
#define ES7210_MIC4_GAIN_REG46             0x46
#define ES7210_MIC1_POWER_REG47            0x47
#define ES7210_MIC2_POWER_REG48            0x48
#define ES7210_MIC3_POWER_REG49            0x49
#define ES7210_MIC4_POWER_REG4A            0x4A
#define ES7210_MIC12_POWER_REG4B           0x4B
#define ES7210_MIC34_POWER_REG4C           0x4C

// Default I2C address (AD1=0, AD0=1)
#define ES7210_I2C_ADDR   0x41

/// Gain values (0~14 => 0dB~37.5dB in 3dB steps)
enum es7210_gain_t : uint8_t {
    ES7210_GAIN_0DB = 0,
    ES7210_GAIN_3DB,
    ES7210_GAIN_6DB,
    ES7210_GAIN_9DB,
    ES7210_GAIN_12DB,
    ES7210_GAIN_15DB,
    ES7210_GAIN_18DB,
    ES7210_GAIN_21DB,
    ES7210_GAIN_24DB,
    ES7210_GAIN_27DB,
    ES7210_GAIN_30DB,
    ES7210_GAIN_33DB,
    ES7210_GAIN_34_5DB,
    ES7210_GAIN_36DB,
    ES7210_GAIN_37_5DB,
};

/// Microphone input selection (bitmask)
enum es7210_mic_select_t : uint8_t {
    ES7210_MIC1 = 0x01,
    ES7210_MIC2 = 0x02,
    ES7210_MIC3 = 0x04,
    ES7210_MIC4 = 0x08,
};

/// Audio data format
enum es7210_fmt_t : uint8_t {
    ES7210_FMT_I2S    = 0,
    ES7210_FMT_LJ     = 1,
    ES7210_FMT_RJ     = 2,
    ES7210_FMT_DSP    = 3,
};

/// Bit width
enum es7210_bits_t : uint8_t {
    ES7210_BIT_16 = 16,
    ES7210_BIT_24 = 24,
    ES7210_BIT_32 = 32,
};

/// Signal type (normal I2S vs TDM)
enum es7210_signal_t : uint8_t {
    ES7210_SIGNAL_I2S = 0,   ///< Normal I2S (2-ch per data line)
    ES7210_SIGNAL_TDM = 1,   ///< TDM (4-ch multiplexed on single data line)
};

/**
 * @brief ES7210 4-ch Audio ADC codec control driver.
 * 
 * Inherits I2C_Device for unified I2C access via I2C_Class.
 * This class handles ONLY the codec register configuration over I2C.
 * I2S data path is managed separately by Mic_Class.
 */
class ES7210_Class : public I2C_Device
{
public:
    ES7210_Class(uint8_t addr = ES7210_I2C_ADDR, I2C_Class* i2c = &In_I2C);

    /// Initialize codec with default settings (slave mode, 16-bit, I2S, 16kHz, MIC1+MIC2)
    bool begin(uint32_t sample_rate = 16000,
               es7210_bits_t bits = ES7210_BIT_16,
               es7210_fmt_t fmt = ES7210_FMT_I2S,
               es7210_signal_t signal = ES7210_SIGNAL_I2S);

    /// De-initialize codec
    void end();

    /// Start ADC conversion
    bool start();

    /// Stop ADC conversion
    bool stop();

    /// Set sample rate (must call before start or re-configure)
    bool setSampleRate(uint32_t rate);

    /// Set bit width
    bool setBits(es7210_bits_t bits);

    /// Set audio format
    bool setFormat(es7210_fmt_t fmt);

    /// Select microphone inputs (bitmask of es7210_mic_select_t)
    bool selectMic(uint8_t mic_mask);

    /// Convenience: set channels (2 or 4), auto-selects MIC1+2 or MIC1~4
    bool setChannels(uint8_t ch);

    /// Set gain for all enabled microphones
    bool setGain(es7210_gain_t gain);

    /// Get current gain value. Returns -1 on error.
    int  getGain();

    /// Set mute state
    bool setMute(bool enable);

    /// Check if chip is present on I2C bus
    bool isPresent();

    /// Print status to Serial
    void printStatus();

    /// Dump all registers for debugging
    void dumpRegisters();

    bool isStarted() const { return _started; }

private:
    // Clock coefficient table entry
    struct CoeffDiv {
        uint32_t mclk;
        uint32_t lrck;
        uint8_t  ss_ds;     ///< 0=single speed, 1=double speed (>=64kHz)
        uint8_t  adc_div;
        uint8_t  dll;
        uint8_t  doubler;
        uint8_t  osr;
        uint32_t lrck_h;
        uint32_t lrck_l;
    };

    static const CoeffDiv  _coeff_table[];
    static const size_t    _coeff_table_size;
    static constexpr uint32_t MCLK_DIV_FRE = 256;

    bool     _started = false;
    uint32_t _sample_rate = 16000;
    es7210_bits_t _bits = ES7210_BIT_16;
    es7210_fmt_t  _fmt     = ES7210_FMT_I2S;
    es7210_signal_t _signal = ES7210_SIGNAL_I2S;
    uint8_t  _mic_mask   = ES7210_MIC1 | ES7210_MIC2;
    es7210_gain_t _gain  = ES7210_GAIN_30DB;
    uint8_t  _clk_reg_save = 0x00;

    /// Update masked bits in a register: reg = (reg & ~mask) | (mask & data)
    bool updateBits(uint8_t reg, uint8_t mask, uint8_t data);

    /// Find clock coefficient index for given mclk/lrck. Returns -1 if not found.
    int  findCoeff(uint32_t mclk, uint32_t lrck);

    /// Reset chip via register
    bool resetChip();
};
