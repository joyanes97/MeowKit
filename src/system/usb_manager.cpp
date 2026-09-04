/**
 * @file usb_manager.cpp
 * @brief USB mode manager — serialises HID / MSC lifecycle on the single OTG port.
 */
#include "usb_manager.h"
#include "usb_msc.h"
#include "usb_hid.h"
#include <Arduino.h>

static usb_mode_t s_active = USB_MODE_NONE;

static void _teardown(usb_mode_t mode)
{
    switch (mode) {
        case USB_MODE_MSC: usb_msc_disable(); break;
        case USB_MODE_HID: usb_hid_disable(); break;
        default: break;
    }
}

static int _bring_up(usb_mode_t mode)
{
    switch (mode) {
        case USB_MODE_MSC:  return usb_msc_enable();
        case USB_MODE_HID:  return usb_hid_enable();
        case USB_MODE_NONE: return 1;
    }
    return 0;
}

int usb_manager_request(usb_mode_t mode)
{
    if (s_active == mode) return 1;

    /* Drain CDC TX before touching the USB stack — prevents data loss and
     * avoids the host seeing a mid-packet disconnect. */
    Serial.flush();
    delay(20);

    usb_mode_t prev = s_active;
    _teardown(prev);
    s_active = USB_MODE_NONE;

    int ok = _bring_up(mode);
    if (ok) s_active = mode;

    Serial.printf("[USB_MGR] %d → %d (%s)\n", (int)prev, (int)mode, ok ? "ok" : "fail");
    return ok;
}

void usb_manager_release(usb_mode_t mode)
{
    if (s_active != mode) return;
    usb_manager_request(USB_MODE_NONE);
}

usb_mode_t usb_manager_get_active(void)
{
    return s_active;
}
