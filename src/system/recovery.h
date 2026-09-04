/**
 * @file recovery.h
 * @brief System recovery utilities.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Erase all NVS settings (brightness, volume, WiFi credentials, etc.)
 * then restart.  Settings revert to firmware defaults on next boot.
 */
void recovery_factory_reset(void);

#ifdef __cplusplus
}
#endif
