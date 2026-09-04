"""
Recovery + conversion for icons corrupted by double-processing.

Background: the .c files were converted twice (TRUE_COLOR→ALPHA twice).
- Original: 4900px × 2 bytes = 9800 bytes (TRUE_COLOR)
- Pass 1 output: 4900px × 3 bytes = 14700 bytes (TRUE_COLOR_ALPHA, correct)
- Pass 2 input: treated 14700 bytes as 7350 2-byte pixels
- Pass 2 output: 7350 × 3 = 22050 bytes in file now (corrupt)

Recovery: for each original pixel k (0..4899), its lo byte was at 1st-pass
position 3k, which ended up at 2nd-pass output position 3*(3k//2)+(3k%2).
Same for hi byte (position 3k+1 → 3*((3k+1)//2)+((3k+1)%2)).
"""
import re

BG_COLOR = 0x4B4E   # RGB565 little-endian: 0x4E lo, 0x4B hi

def extract_hex(text):
    """Return all 0xNN values from a C data array body."""
    return [int(x, 16) for x in re.findall(r'0x[0-9A-Fa-f]{2}', text)]

def recover_and_convert(path):
    with open(path, 'r', encoding='utf-8') as f:
        txt = f.read()

    m = re.search(r'(\[\]\s*=\s*\{)(.*?)(\};)', txt, re.DOTALL)
    if not m:
        print("ERROR: array not found in " + path)
        return

    data_raw = m.group(2)
    out_bytes = extract_hex(data_raw)
    n_out = len(out_bytes)

    # Determine mode based on byte count
    if n_out == 9800:
        # Original TRUE_COLOR – do a clean first conversion
        n_px = 4900
        src = out_bytes
        new_bytes = []
        for i in range(0, n_px * 2, 2):
            lo, hi = src[i], src[i+1]
            alpha = 0x00 if ((hi << 8) | lo) == BG_COLOR else 0xFF
            new_bytes.extend([lo, hi, alpha])
        print("{}: clean convert {} -> {} bytes".format(
            path.split("\\")[-1], n_out, len(new_bytes)))

    elif n_out == 14700:
        # Already correctly converted, just rebuild
        new_bytes = list(out_bytes)
        print("{}: already correct ({} bytes), rebuilding".format(
            path.split("\\")[-1], n_out))

    elif n_out == 22050:
        # Double-processed: recover using byte-position formula
        n_px = 4900
        new_bytes = []
        for k in range(n_px):
            p_lo = 3 * k
            idx_lo = 3 * (p_lo // 2) + (p_lo % 2)
            p_hi = 3 * k + 1
            idx_hi = 3 * (p_hi // 2) + (p_hi % 2)
            lo = out_bytes[idx_lo]
            hi = out_bytes[idx_hi]
            alpha = 0x00 if ((hi << 8) | lo) == BG_COLOR else 0xFF
            new_bytes.extend([lo, hi, alpha])
        print("{}: recovered from double-pass {} -> {} bytes".format(
            path.split("\\")[-1], n_out, len(new_bytes)))

    else:
        print("ERROR: unexpected byte count {} in {}".format(n_out, path))
        return

    # Rebuild C file
    prefix  = txt[:m.start(2)]
    suffix  = txt[m.end(3):]

    rows = []
    for i in range(0, len(new_bytes), 16):
        row = ','.join('0x{:02X}'.format(b) for b in new_bytes[i:i+16])
        rows.append('    ' + row + ',')
    new_data = '\n' + '\n'.join(rows) + '\n'

    # Fix include path and ensure cf = TRUE_COLOR_ALPHA
    new_suffix = re.sub(r'LV_IMG_CF_TRUE_COLOR\b(?!_ALPHA)',
                        'LV_IMG_CF_TRUE_COLOR_ALPHA', suffix)
    fixed_prefix = prefix.replace('#include "ui.h"', '#include "../ui.h"')

    result = fixed_prefix + new_data + '};' + new_suffix
    with open(path, 'w', encoding='utf-8') as f:
        f.write(result)

base = r"C:\Users\zmm04\Desktop\mk_firmware_0614\src\ui\images"
recover_and_convert(base + r"\ui_img_dino_png.c")
recover_and_convert(base + r"\ui_img_vu_meter_png.c")
