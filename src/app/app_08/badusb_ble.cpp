/* BleKeyboard isolation TU — kept separate so BleKeyboard.h's KeyReport /
 * KEY_F13..F24 definitions never collide with Arduino USB HID headers. */
#include "badusb_ble.h"
#include <Arduino.h>
#include <BleKeyboard.h>

static BleKeyboard s_ble("MeowKit BadUSB", "MeowKit", 100);
static bool s_started = false;

void bu_ble_begin()
{
    if (!s_started) { s_ble.begin(); s_started = true; }
}

void bu_ble_end()
{
    /* BleKeyboard::end() is empty — it never deregisters GATT services.
     * Calling begin() again creates a new server with new characteristic
     * handles that the host is NOT subscribed to → notify() silently drops.
     * Keep the BLE stack running once started. */
    (void)0;
}

bool bu_ble_connected()
{
    return s_started && s_ble.isConnected();
}

void bu_ble_press(uint8_t k)     { if (s_started) s_ble.press(k); }
void bu_ble_release(uint8_t k)   { if (s_started) s_ble.release(k); }
void bu_ble_release_all()        { if (s_started) s_ble.releaseAll(); }

/* BleKeyboard::write(c) calls press(c)+release(c) with no inter-report delay.
 * Bluedroid queues notifications asynchronously — without a gap the "released"
 * report can be transmitted before the host processes the "pressed" one.
 * Use explicit press / delay / release so each notification has time to be
 * delivered before the next one is queued. */
void bu_ble_write(uint8_t c) {
    if (!s_started) return;
    s_ble.press(c);
    delay(8);
    s_ble.release(c);
    delay(4);
}

void bu_ble_print(const char* s) {
    if (!s_started) return;
    while (*s) {
        s_ble.press((uint8_t)*s);
        delay(8);
        s_ble.release((uint8_t)*s);
        delay(4);
        ++s;
    }
}
