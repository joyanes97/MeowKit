/**
 * @file recovery.cpp
 * @brief System recovery utilities.
 */
#include "recovery.h"
#include "persist.h"
#include <Arduino.h>
#include <esp_system.h>

extern "C" void recovery_factory_reset(void)
{
    Serial.println("[Recovery] Factory reset — erasing NVS...");
    Serial.flush();
    persist_erase_all();
    esp_restart();
}
