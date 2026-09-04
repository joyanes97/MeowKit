#include "splash_screen.h"
#include "splash_gif.h"
#include <AnimatedGIF.h>
#include <LovyanGFX.hpp>

static constexpr int SCR_W = 320;
static constexpr int SCR_H = 240;


static LGFX_Class* s_lcd   = nullptr;
static int         s_gif_x = 0;
static int         s_gif_y = 0;
static uint16_t    s_linebuf[SCR_W];

static void GIFDraw(GIFDRAW* pDraw)
{
    if (!s_lcd) return;

    uint8_t*  s         = pDraw->pPixels;
    uint16_t* usPalette = pDraw->pPalette;
    int       iWidth    = pDraw->iWidth;
    if (iWidth > SCR_W) iWidth = SCR_W;
    const int y = s_gif_y + pDraw->iY + pDraw->y;

    if (pDraw->ucDisposalMethod == 2) {
        for (int x = 0; x < iWidth; x++) {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    if (pDraw->ucHasTransparency) {
        uint8_t* pEnd = s + iWidth;
        const uint8_t ti = pDraw->ucTransparent;
        int x = 0;
        while (x < iWidth) {
            uint16_t* d = s_linebuf;
            int count = 0;
            while (s < pEnd && *s != ti) { *d++ = usPalette[*s++]; count++; }
            if (count)
                s_lcd->pushImage(s_gif_x + pDraw->iX + x, y, count, 1, s_linebuf);
            x += count;
            while (s < pEnd && *s == ti) { s++; x++; }
        }
    } else {
        for (int x = 0; x < iWidth; x++)
            s_linebuf[x] = usPalette[*s++];
        s_lcd->pushImage(s_gif_x + pDraw->iX, y, iWidth, 1, s_linebuf);
    }
}

void SplashScreen::show(LGFX_Class& lcd)
{
    /* Guarantee backlight is off — defense against any earlier state change. */
    lcd.setBrightness(0);

    /* GRAM is already black (devices.cpp filled it before calling us).
     * Do NOT draw the home-screen background: its colors bleed through
     * transparent GIF pixels and produce a visible flash before the animation.
     * GIF plays on solid black; transparent areas stay black. */

    if (splash_gif_len < 10) {
        delay(1500);
        return;
    }

    // static: GIFIMAGE ≈ 24 KB — too large for the 16 KB task stack
    static AnimatedGIF gif;
    gif.begin(BIG_ENDIAN_PIXELS);

    if (!gif.openFLASH((uint8_t*)splash_gif_data, (int)splash_gif_len, GIFDraw)) {
        return;
    }

    s_lcd = &lcd;
    const int gw = gif.getCanvasWidth();
    const int gh = gif.getCanvasHeight();
    s_gif_x = (gw < SCR_W) ? (SCR_W - gw) / 2 : 0;
    s_gif_y = (gh < SCR_H) ? (SCR_H - gh) / 2 : 0;

    /* Render first frame while backlight is still off — user never sees the
     * background-only state or a partial first frame. */
    int delayMs = 0;
    if (!gif.playFrame(false, &delayMs)) {
        gif.close();
        s_lcd = nullptr;
        return;
    }

    /* Fade in: reveals first GIF frame on clean black background */
    for (int b = 0; b <= 255; b += 8) {
        lcd.setBrightness((uint8_t)b);
        delay(5);
    }
    lcd.setBrightness(255);

    /* Play remaining frames with manual timing so a corrupt delay can't block.
     * Cap total playback at 15 s; cap individual frame waits at 1 s. */
    const uint32_t gif_deadline = millis() + 15000;
    while (gif.playFrame(false, &delayMs)) {
        if (millis() >= gif_deadline) break;
        uint32_t wait = (delayMs > 0 && delayMs < 1000) ? (uint32_t)delayMs : 50;
        delay(wait);
    }

    gif.close();
    s_lcd = nullptr;

    /* Fade out: GIF → black */
    for (int b = 255; b >= 0; b -= 8) {
        lcd.setBrightness((uint8_t)b);
        delay(10);
    }
    lcd.setBrightness(0);

    /* Clear frame buffer while backlight is off.
     * Prevents GIF pixels from flashing through when LVGL init restores brightness. */
    lcd.fillScreen(0x0000);
}
