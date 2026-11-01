#!/usr/bin/env python3
# Converts PNG images to single-texture TPL files (RGB5A3), for layout picture panes.
# Usage: png2tpl.py <size> <out dir> <png>...   (each image is resized to size x size)
import os
import struct
import sys

from PIL import Image


def rgb5a3(r, g, b, a):
    if a >= 0xe0:
        return 0x8000 | (r >> 3) << 10 | (g >> 3) << 5 | (b >> 3)
    return (a >> 5) << 12 | (r >> 4) << 8 | (g >> 4) << 4 | (b >> 4)


def encode(im):
    w, h = im.size
    px = im.load()
    out = bytearray()
    for by in range(0, h, 4):
        for bx in range(0, w, 4):
            for y in range(by, by + 4):
                for x in range(bx, bx + 4):
                    out += struct.pack('>H', rgb5a3(*px[x, y]))
    return bytes(out)


def tpl(im):
    w, h = im.size
    data = encode(im)
    header = struct.pack('>III', 0x0020AF30, 1, 0x0C)
    table = struct.pack('>II', 0x14, 0)
    image = struct.pack('>HHIIIIIIfBBBB', h, w, 5, 0x40, 0, 0, 1, 1, 0.0, 0, 0, 0, 0)
    body = header + table + image
    return body + b'\0' * (0x40 - len(body)) + data


def main():
    size, out_dir = int(sys.argv[1]), sys.argv[2]
    for path in sys.argv[3:]:
        im = Image.open(path).convert('RGBA').resize((size, size), Image.LANCZOS)
        name = os.path.splitext(os.path.basename(path))[0]
        with open(os.path.join(out_dir, f'rank_{name}.tpl'), 'wb') as f:
            f.write(tpl(im))


if __name__ == '__main__':
    main()
