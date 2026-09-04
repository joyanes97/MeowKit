/**
 * @file Mic_Class.hpp
 * @brief Microphone recording class for MeowKit (ESP32-S3 + ES7210)
 * 
 * Design referenced from M5Unified Mic_Class, simplified and adapted:
 * - Uses ESP-IDF legacy I2S driver (driver/i2s.h) for audio data path
 * - Uses ES7210_Class (I2C_Device) for codec register control
 * - FreeRTOS background task for continuous DMA read + processing
 * - Oversampling, noise filter, auto-DC-offset removal
 *
 * NOTE: No custom I2S driver is needed — the ESP-IDF i2s driver handles
 * DMA, clocking, and pin mux. This class is a thin orchestration layer.
 */
#pragma once

#include "ES7210_Class.hpp"
#include "../config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "driver/i2s.h"
#include <cstdint>
#include <cstddef>

/**
 * @brief Mic recording configuration.
 */
struct mic_config_t
{
    // ── I2S pin mapping (defaults from config.h) ──
    int pin_mclk    = MCLKPIN;      ///< Master clock
    int pin_bclk    = BCLKPIN;      ///< Bit clock
    int pin_ws      = WSPIN;        ///< Word select / LRCK
    int pin_data_in = DIPIN;        ///< Data in (from ES7210 SDOUT)

    // ── Recording parameters ──
    uint32_t sample_rate    = 16000; ///< Sampling rate in Hz
    uint8_t  over_sampling  = 2;     ///< Oversampling factor (1~8)
    uint8_t  magnification  = 16;    ///< Input gain multiplier
    uint8_t  noise_filter   = 0;     ///< Noise filter coefficient (0=off, 1~255)
    bool     stereo         = false; ///< true = stereo, false = mono

    // ── DMA tuning ──
    size_t   dma_buf_len    = 256;   ///< Samples per DMA buffer
    size_t   dma_buf_count  = 6;     ///< Number of DMA buffers

    // ── Background task ──
    uint8_t  task_priority     = 2;
    uint8_t  task_pinned_core  = 1;  ///< Pin to core 1 (keep core 0 for UI)

    // ── I2S port (ESP32-S3 has I2S_NUM_0 and I2S_NUM_1) ──
    i2s_port_t i2s_port = I2S_NUM_1; ///< Default to port 1 (port 0 may be used by speaker)
};

/**
 * @brief Mic recording class.
 *
 * Usage:
 * @code
 *   Mic_Class mic;
 *   mic.config().sample_rate = 16000;
 *   mic.begin();                           // starts background task
 *   int16_t buf[320];
 *   mic.record(buf, 320, 16000);           // blocks until filled
 *   mic.end();
 * @endcode
 */
class Mic_Class
{
public:
    Mic_Class() = default;
    ~Mic_Class() { end(); }

    /// Access configuration (modify before begin())
    mic_config_t& config() { return _cfg; }
    const mic_config_t& config() const { return _cfg; }

    /// Set the ES7210 codec instance (must be called before begin if not default)
    void setCodec(ES7210_Class* codec) { _codec = codec; }

    /**
     * @brief Start the I2S driver and recording background task.
     * 
     * Also initialises and starts the ES7210 codec if a codec pointer is set.
     * @return true on success
     */
    bool begin();

    /**
     * @brief Stop recording and release I2S driver.
     */
    void end();

    /// Is the background task running?
    bool isRunning() const { return _task_running; }

    /// Is a recording operation in progress?
    bool isRecording() const { return _is_recording; }

    /**
     * @brief Record 16-bit signed PCM samples (blocking).
     * @param data   destination buffer
     * @param length number of samples to record
     * @param sample_rate  desired sample rate (will reconfigure if different)
     * @param stereo true for interleaved stereo
     * @return true if recording started, false on error
     */
    bool record(int16_t* data, size_t length, uint32_t sample_rate = 0, bool stereo = false);

    /**
     * @brief Record 8-bit unsigned PCM samples (blocking).
     */
    bool record(uint8_t* data, size_t length, uint32_t sample_rate = 0, bool stereo = false);

private:
    // ── Recording info (double-buffered for seamless recording) ──
    struct recording_info_t {
        void*  data      = nullptr;
        size_t length    = 0;
        bool   is_stereo = false;
        bool   is_16bit  = false;
    };

    recording_info_t _rec_info[2];
    volatile bool    _rec_flip = false;

    // ── I2S state ──
    bool _i2s_installed = false;

    // ── Codec ──
    ES7210_Class*    _codec = nullptr;

    // ── Config ──
    mic_config_t     _cfg;
    uint32_t         _rec_sample_rate = 0;

    // ── Task state ──
    volatile bool    _task_running  = false;
    volatile bool    _is_recording  = false;
    TaskHandle_t     _task_handle   = nullptr;
    SemaphoreHandle_t _task_semaphore = nullptr;

    // ── DSP state ──
    int32_t          _offset = 0;   ///< DC offset tracking

    // ── Internal ──
    bool     _setup_i2s();
    void     _uninstall_i2s();
    bool     _rec_raw(void* data, size_t length, bool is_16bit,
                      uint32_t sample_rate, bool stereo);
    uint32_t _calc_rec_rate() const;

    static void mic_task(void* args);
};
