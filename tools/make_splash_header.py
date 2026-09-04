#!/usr/bin/env python3
"""
Convert GIF -> per-frame JPEG C header for splash screen.
No AnimatedGIF library needed — LovyanGFX drawJpg() handles decoding.

Usage:
    python tools/make_splash_header.py  <input.gif>  [jpeg_quality]  > src/splash/splash_gif.h

Default quality: 65  (good balance for cartoon/logo content)
Background fill: #1A1A1A (matches COL_BG in splash_screen.cpp)
"""
import sys, os, io
from PIL import Image, ImageSequence

BG_COLOR = (26, 26, 26)   # #1A1A1A


def main():
    path    = sys.argv[1] if len(sys.argv) > 1 else "logo_small.gif"
    quality = int(sys.argv[2]) if len(sys.argv) > 2 else 65

    if not os.path.isfile(path):
        print(f"Error: not found: {path}", file=sys.stderr); sys.exit(1)

    src = Image.open(path)
    W, H = src.size

    frames_jpg, delays = [], []
    canvas = Image.new("RGB", (W, H), BG_COLOR)

    for f in ImageSequence.Iterator(src):
        dur  = f.info.get("duration", 60)
        disp = f.info.get("disposal", 0)

        rgba = f.convert("RGBA")
        comp = canvas.copy()
        comp.paste(rgba, mask=rgba.split()[3])   # alpha-composite onto canvas

        buf = io.BytesIO()
        comp.save(buf, format="JPEG", quality=quality, optimize=True)
        frames_jpg.append(buf.getvalue())
        delays.append(max(dur, 16))   # cap at 16 ms minimum

        if disp == 2:                 # restore to background
            canvas = Image.new("RGB", (W, H), BG_COLOR)
        elif disp != 3:               # leave / unspecified
            canvas = comp

    total_kb = sum(len(d) for d in frames_jpg) / 1024
    print(f"// {len(frames_jpg)} frames  {W}x{H}  {total_kb:.1f} KB JPEG total", file=sys.stderr)
    if total_kb > 600:
        print("Warning: still large — consider reducing quality arg (e.g. 45)", file=sys.stderr)

    # ── emit header ──────────────────────────────────────────────────────────
    print("#pragma once")
    print("#include <stdint.h>")
    print()
    print(f"// Source: {os.path.basename(path)}  {len(frames_jpg)} frames  {W}x{H}  {total_kb:.1f} KB")
    print(f"#define SPLASH_FRAME_COUNT {len(frames_jpg)}")   # must be #define — used in #if
    print(f"static constexpr int SPLASH_FRAME_W = {W};")
    print(f"static constexpr int SPLASH_FRAME_H = {H};")
    print()

    print("static const uint16_t splash_delays[] = {")
    print("    " + ", ".join(str(d) for d in delays))
    print("};")
    print()

    print("static const uint32_t splash_sizes[] = {")
    print("    " + ", ".join(str(len(d)) for d in frames_jpg))
    print("};")
    print()

    for i, data in enumerate(frames_jpg):
        print(f"static const uint8_t splash_frame_{i:02d}[] = {{")
        for j in range(0, len(data), 16):
            chunk = data[j:j+16]
            print("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
        print("};")
        print()

    print("static const uint8_t* const splash_frames[] = {")
    for i in range(len(frames_jpg)):
        print(f"    splash_frame_{i:02d},")
    print("};")


if __name__ == "__main__":
    main()
