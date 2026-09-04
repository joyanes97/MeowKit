/* Thin C-style wrapper around BleKeyboard so its header (which collides
 * with Arduino's USBHIDKeyboard on KeyReport / KEY_F13..F24) stays isolated
 * to a single translation unit. */
#pragma once
#include <stdint.h>

void bu_ble_begin();
void bu_ble_end();
bool bu_ble_connected();
void bu_ble_press(uint8_t k);
void bu_ble_release(uint8_t k);
void bu_ble_release_all();
void bu_ble_write(uint8_t c);
void bu_ble_print(const char* s);
