/**
 * @file  devices.h
 * @brief MeowKit BSP — Hardware Abstraction Layer
 *
 * Aggregates ALL on-board peripherals into a single DEVICES structure.
 * Core peripherals are value members (always available after init()).
 * Optional subsystems (audio, IR) are initialized on demand by apps.
 *
 * Hardware inventory:
 *   Display   ST7789 320×240 SPI IPS + PWM backlight
 *   Touch     FT6336 capacitive (I²C 0x38)
 *   PMU       AXP173 (I²C 0x34) — Li-ion 1400 mAh, USB-C charge
 *   IO Exp    PCA9557 8-bit (I²C 0x19)
 *   RTC       PCF8563 (I²C 0x51)
 *   IMU       BMI270 6-axis + BMM150 3-axis mag (I²C 0x68)
 *   LED       WS2812B-2020 (GPIO 38)
 *   Buttons   A (GPIO 6), B (GPIO 4)
 *   Joystick  5-way — Up/Down/Left/Right/Press
 *   Audio     ES8311 DAC → NS4150B amp → 1 W speaker
 *             ES7210 ADC ← ZTS6216 MEMS mic
 *   IR        940 nm TX (GPIO 7) / 950 nm RX (GPIO 5)
 *   SD        SDMMC 1-bit (CLK 47, CMD 48, D0 21)
 *   USB       Native USB-OTG (DP 20, DN 19) — CDC / HID / MSC
 */
#pragma once

/* ── Core peripheral drivers ────────────────────────────── */
#include "porting/touch.hpp"          // CTP_Class (FT6336)
#include "porting/display.hpp"        // LGFX_Class (ST7789 + LovyanGFX)
#include "button/Button.hpp"          // Button_Class / Button_Device
#include "power/AXP173.hpp"           // AXP173_Class PMU
#include "io_exp/PCA9557.hpp"         // PCA9557_Class IO expander
#include "rtc/PCF8563.hpp"            // PCF8563_Class RTC
#include "led/WS2812B.hpp"            // WS2812B_Class LED
#include "imu/IMU_Class.hpp"          // BMI270 + BMM150
#include "audio/Speaker_Class.hpp"    // ES8311 DAC + I2S TX
#include "audio/ES8311_Class.hpp"
#include "audio/Mic_Class.hpp"        // ES7210 ADC + I2S RX
/* ── Extended BSP — connectivity & storage ──────────────── */
#include "wifi/WiFi_Class.hpp"        // WiFi_Class (STA / AP / scan)
#include "sdmmc/SDMMC_Class.hpp"      // SDMMC_Class (multi-freq SD)
#include "config.h"

/**
 * @class DEVICES
 * @brief Top-level hardware aggregator — one instance for the entire system.
 *
 * Lifecycle:
 *   1. Launcher creates DEVICES on the heap
 *   2. init() powers up and configures core peripherals
 *   3. Pointer is passed to Mooncake apps via constructor
 *   4. Apps use members directly (e.g. device->Lcd, device->button)
 */
class DEVICES
{
public:
    
    LGFX_Class      Lcd;              ///< ST7789 display (LovyanGFX)
    CTP_Class       ctp;              ///< FT6336 capacitive touch
    Button_Device   button;           ///< A / B + 4-dir joystick
    AXP173_Class    pmu;              ///< Power management IC
    PCA9557_Class   io_exp;           ///< I²C IO expander
    PCF8563_Class   rtc;              ///< Real-time clock
    WS2812B_Class   led;              ///< WS2812B status LED
    IMU_Class       imu;              ///< 9-axis motion sensor
    Speaker_Class   speaker;          ///< ES8311 DAC → amp → speaker
    Mic_Class       mic;              ///< ES7210 ADC ← MEMS microphone
    WiFi_Class      wifi;             ///< WiFi STA / AP / scan manager
    SDMMC_Class     sd;               ///< SDMMC with auto-freq negotiation

    bool init();
};