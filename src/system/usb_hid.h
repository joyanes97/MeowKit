/**
 * @file usb_hid.h
 * @brief USB HID system service — owns TinyUSB HID statics and lifecycle.
 *
 * HID instances (keyboard / mouse / consumer-control) must be file-scope
 * before USB.begin() so TinyUSB can build the composite descriptor at
 * static-init time.  This file centralises that ownership so the
 * usb_manager can enable/disable HID without touching app layer code.
 *
 * Callers (BadUSB app) access devices through the typed C++ accessors.
 * All lifecycle is driven through usb_manager_request() / _release().
 */
#pragma once

#ifdef __cplusplus
#if ARDUINO_USB_MODE == 0
#include <USBHIDKeyboard.h>
// USBHIDMouse.h (Arduino framework) unconditionally re-defines MOUSE_LEFT /
// MOUSE_RIGHT / MOUSE_MIDDLE / MOUSE_FORWARD / MOUSE_ALL — the same values
// that BleMouse.h (BLE app) already defined with #ifndef guards.  Undef them
// here at the USB system-integration boundary so the compiler sees only one
// definition.  Values are numerically identical; behaviour is unchanged.
#undef MOUSE_LEFT
#undef MOUSE_RIGHT
#undef MOUSE_MIDDLE
#undef MOUSE_BACK
#undef MOUSE_FORWARD
#undef MOUSE_ALL
#include <USBHIDMouse.h>
#include <USBHIDConsumerControl.h>
#endif

extern "C" {
#endif

/** Called by usb_manager — do not call directly. */
int  usb_hid_enable(void);
void usb_hid_disable(void);
int  usb_hid_is_active(void);

/** True if the TinyUSB HID device is mounted by the host. */
int  usb_hid_connected(void);

/** Keyboard LED state from host (NUM_LOCK / CAPS_LOCK / SCROLL_LOCK bits). */
uint8_t usb_hid_leds(void);

#ifdef __cplusplus
}

#if ARDUINO_USB_MODE == 0
USBHIDKeyboard        & usb_hid_kbd(void);
USBHIDMouse           & usb_hid_mouse(void);
USBHIDConsumerControl & usb_hid_consumer(void);
#endif
#endif
