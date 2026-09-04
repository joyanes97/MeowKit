/**
 * @file  lv_port_fs.h
 * @brief LVGL filesystem driver port — SD_MMC (drive letter 'S:')
 *
 * Maps LVGL lv_fs operations to Arduino SD_MMC API.
 * Usage:  lv_img_set_src(img, "S:apps/icon/app_01.png");
 */

#ifndef LV_PORT_FS_H
#define LV_PORT_FS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Register LVGL filesystem driver for SD_MMC (call after SD mount) */
void lv_fs_fatfs_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_FS_H*/
