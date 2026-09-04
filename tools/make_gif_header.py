#!/usr/bin/env python3
"""
Convert a GIF file to a C header for src/splash/splash_gif.h

Usage:
    python tools/make_gif_header.py  animation.gif  > src/splash/splash_gif.h

The script validates the GIF89a signature and prints file size info to stderr.
"""
import sys
import os

def main():
    if len(sys.argv) < 2:
        print(f"Usage: python {sys.argv[0]} <input.gif>", file=sys.stderr)
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.isfile(path):
        print(f"Error: file not found: {path}", file=sys.stderr)
        sys.exit(1)

    with open(path, "rb") as f:
        data = f.read()

    if not (data[:6] in (b"GIF89a", b"GIF87a")):
        print(f"Error: {path} is not a valid GIF file", file=sys.stderr)
        sys.exit(1)

    size_kb = len(data) / 1024
    print(f"Source: {os.path.basename(path)}  ({size_kb:.1f} KB = {len(data)} bytes)", file=sys.stderr)
    if size_kb > 300:
        print("Warning: GIF is large (>300 KB). Consider optimising with gifsicle or ezgif.com.", file=sys.stderr)

    print("#pragma once")
    print("#include <stdint.h>")
    print()
    print(f"// Source: {os.path.basename(path)}  ({len(data)} bytes)")
    print("static const uint8_t splash_gif_data[] = {")
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        print("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    print("};")
    print(f"static const size_t splash_gif_len = {len(data)};")

if __name__ == "__main__":
    main()
