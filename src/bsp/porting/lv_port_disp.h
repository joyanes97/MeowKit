/**
 * @file  lv_port_disp.h
 * @brief LVGL display driver port — ST7789 320×240 via LovyanGFX SPI
 *
 * Uses PSRAM double-buffered full-frame (2 × 320×240×2 = 300 KB).
 * Flush callback writes pixels through LovyanGFX writePixels().
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include "lvgl.h"
#include "display.hpp"

/* Display geometry */
#define LVGL_USE_PSRAM     1
#define MY_DISP_HOR_RES    320
#define MY_DISP_VER_RES    240

/** Initialize LVGL display driver (call after lv_init) */
void lv_port_disp_init(LGFX_Class* lcd);

/** Enable/disable screen update (pause flushing during heavy operations) */
void disp_enable_update(void);
void disp_disable_update(void);

#endif /*LV_PORT_DISP_H*/
