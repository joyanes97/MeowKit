/**
 * @file usb_hid.cpp
 * @brief USB HID system service implementation.
 *
 * File-scope HID statics register TinyUSB descriptors at static-init time
 * (before USB.begin()).  enable/disable only starts/stops the interface;
 * the descriptor is always present in the composite device.
 */
#include "usb_hid.h"
#include <Arduino.h>

#if ARDUINO_USB_MODE == 0
#include <USB.h>

extern "C" bool tud_mounted(void) __attribute__((weak));

static USBHIDKeyboard        s_kbd;
static USBHIDMouse           s_mouse;
static USBHIDConsumerControl s_consumer;
static volatile uint8_t      s_leds   = 0;
static bool                  s_began  = false;
static bool                  s_active = false;

int usb_hid_enable(void)
{
    if (s_active) return 1;
    if (!s_began) {
        s_kbd.begin();
        s_mouse.begin();
        s_consumer.begin();
        s_kbd.onEvent(ARDUINO_USB_HID_KEYBOARD_LED_EVENT,
            [](void*, esp_event_base_t, int32_t, void* data) {
                s_leds = ((arduino_usb_hid_keyboard_event_data_t*)data)->leds;
            });
        s_began = true;
    }
    s_active = true;
    Serial.println("[USB_HID] enabled");
    return 1;
}

void usb_hid_disable(void)
{
    if (!s_active) return;
    s_kbd.releaseAll();
    s_mouse.release(0x07);   /* LEFT | RIGHT | MIDDLE */
    s_consumer.release();
    s_active = false;
    Serial.println("[USB_HID] disabled");
}

int  usb_hid_is_active(void)  { return s_active ? 1 : 0; }
int  usb_hid_connected(void)  { return (tud_mounted ? tud_mounted() : false) ? 1 : 0; }
uint8_t usb_hid_leds(void)    { return s_leds; }

USBHIDKeyboard        & usb_hid_kbd(void)      { return s_kbd; }
USBHIDMouse           & usb_hid_mouse(void)    { return s_mouse; }
USBHIDConsumerControl & usb_hid_consumer(void) { return s_consumer; }

#else  /* ARDUINO_USB_MODE != 0 — stub out for CDC-only builds */

int     usb_hid_enable(void)    { return 0; }
void    usb_hid_disable(void)   {}
int     usb_hid_is_active(void) { return 0; }
int     usb_hid_connected(void) { return 0; }
uint8_t usb_hid_leds(void)      { return 0; }

#endif
