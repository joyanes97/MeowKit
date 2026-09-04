/**
 * @file device_status.h
 * @brief Aggregated device-state snapshot for status bar / settings UI.
 *
 * Call device_status_update() each second (or on demand).
 * Read via device_status_get() — returns a pointer to a static struct.
 *
 * Fields:
 *   wifi_connected   STA link is up
 *   wifi_ssid        Connected SSID (or "" if disconnected)
 *   wifi_ip          IP address string (or "")
 *   bt_active        Any BT/BLE profile is running
 *   lcd_brightness   0-100 (last value from settings_bridge)
 *   led_brightness   0-100 (last set LED intensity)
 *   btn_sound        Button-click sound enabled
 *   sd_present       SD card currently inserted & mounted
 *   battery_pct      0-100 from AXP173 coulometer
 *   charging         USB-VBUS detected + charging
 */
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
#include "../bsp/devices.h"
extern "C" {
#endif

typedef struct {
    bool  wifi_connected;
    char  wifi_ssid[33];
    char  wifi_ip[16];
    bool  bt_active;
    int   lcd_brightness;
    int   led_brightness;
    bool  btn_sound;
    bool  sd_present;
    int   battery_pct;
    bool  charging;
} DeviceStatus;

/** Refresh all status fields from hardware. Call ~1 Hz. */
void device_status_update(DEVICES* dev);

/** Return pointer to the current (last-updated) snapshot. Never NULL. */
const DeviceStatus* device_status_get(void);

/** Manually set btn_sound flag (persisted by caller). */
void device_status_set_btn_sound(bool on);
void device_status_set_led_brightness(int pct);

#ifdef __cplusplus
}
#endif
