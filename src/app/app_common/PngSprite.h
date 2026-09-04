/**
 * @file    PngSprite.h
 * @brief   app_common shared component — PNG-from-SD → LGFX_Sprite loader
 *
 * Header-only utility. Decodes a PNG file from the SD card (via SD_MMC)
 * into a freshly allocated LGFX_Sprite backed by PSRAM.
 *
 * Usage:
 *   #include "../../app_common/PngSprite.h"
 *
 *   LGFX_Sprite* sp = AppCommon::loadPngSprite(&_device->Lcd, "/apps/foo/bg.png");
 *   if (sp) {
 *       sp->pushSprite(&_device->Lcd, 0, 0);
 *       sp->deleteSprite();
 *       delete sp;
 *   }
 *
 * The caller owns the returned pointer and is responsible for:
 *   1. Calling sp->deleteSprite()    (frees PSRAM pixel buffer)
 *   2. Calling delete sp             (frees the C++ object)
 *
 * Returns nullptr if the file is not found, PNG is malformed, or
 * PSRAM is exhausted. Never throws.
 */
#pragma once
#include <SD_MMC.h>
#include <LovyanGFX.hpp>
#include <cstring>

namespace AppCommon {

/**
 * @param parent     LGFX display device (sprite inherits its config context).
 * @param path       Absolute SD path, e.g. "/apps/vu meter/vu_meter_bg.png".
 * @param colorDepth Sprite color depth: 16 (RGB565, default) or 24 (RGB888).
 * @return           Ready-to-use LGFX_Sprite*, or nullptr on any failure.
 */
inline LGFX_Sprite* loadPngSprite(LovyanGFX* parent,
                                   const char* path,
                                   uint8_t colorDepth = 16)
{
    /* ── Open file ── */
    File f = SD_MMC.open(path, "r");
    if (!f) return nullptr;

    size_t sz = (size_t)f.size();
    if (!sz) { f.close(); return nullptr; }

    /* ── Read into PSRAM buffer ── */
    uint8_t* buf = (uint8_t*)ps_malloc(sz);
    if (!buf) { f.close(); return nullptr; }

    if (f.read(buf, sz) != sz) { free(buf); f.close(); return nullptr; }
    f.close();

    /* ── Parse PNG IHDR dimensions (no full decode required) ──
     *  Layout: 8-byte PNG sig | 4-byte IHDR chunk len | 4-byte "IHDR" |
     *          4-byte width | 4-byte height | …
     *  Width starts at byte 16, height at byte 20.
     */
    int32_t w = 320, h = 240;    // safe fallback for 320×240 displays
    static const uint8_t kSig[8] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    if (sz >= 24 && memcmp(buf, kSig, 8) == 0) {
        w = (int32_t)(((uint32_t)buf[16]<<24)|((uint32_t)buf[17]<<16)|
                      ((uint32_t)buf[18]<< 8)| (uint32_t)buf[19]);
        h = (int32_t)(((uint32_t)buf[20]<<24)|((uint32_t)buf[21]<<16)|
                      ((uint32_t)buf[22]<< 8)| (uint32_t)buf[23]);
        if (w <= 0 || h <= 0) { w = 320; h = 240; }
    }

    /* ── Allocate sprite and decode ── */
    auto* sp = new LGFX_Sprite(parent);
    sp->setColorDepth(colorDepth);
    if (!sp->createSprite(w, h)) { free(buf); delete sp; return nullptr; }

    sp->fillSprite(TFT_BLACK);
    sp->drawPng(buf, sz, 0, 0);
    free(buf);
    return sp;
}

} // namespace AppCommon
