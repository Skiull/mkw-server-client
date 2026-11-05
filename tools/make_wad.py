#!/usr/bin/env python3
"""Pack out/<profile>/boot.dol into an installable Wii channel (.wad).

The channel is a small libogc menu (channel/) with a "Launch Game" button. It
embeds boot.dol and starts it the way the Homebrew Channel does, which boots
Mario Kart Wii from the disc with the mkw-server-client payload.

Nothing from Nintendo is stored in this repo. At build time the script takes:
  - the channel banner: opening.bnr from your own MKW disc image (via DolphinTool)
  - the certificate chain: cert.sys from your Dolphin user folder (or --cert-chain)
  - the Wii common key: read from your Dolphin.exe, or given with --common-key
    (16-byte file, hex string, or a BootMii keys.bin)

The ticket and TMD are fakesigned, so a real Wii needs a WAD manager that
accepts them (e.g. Wii Mod Lite / YAWM ModMii Edition on a patched IOS).
"""

import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

# SHA-256 of the Wii common key. Used only to confirm we found the right key.
COMMON_KEY_SHA256 = 'de38aeab4fe0c36d828a47e6fd315100e7ce234d3b00aa25e6ad6f5ff2824af8'

TITLE_ID = 0x000100014D4B5348  # 00010001-MKSH
IOS_ID = 0x000000010000003A    # IOS58, which the loader requires
CHANNEL_NAME = 'MKW Self-host'


def align(n, a=0x40):
    return (n + a - 1) // a * a


def pad(data, a=0x40):
    return data + b'\0' * (align(len(data), a) - len(data))


def aes_cbc_encrypt(key, iv, data):
    enc = Cipher(algorithms.AES(key), modes.CBC(iv)).encryptor()
    return enc.update(data) + enc.finalize()


# --- Common key -----------------------------------------------------------

def check_key(key):
    return key is not None and hashlib.sha256(key).hexdigest() == COMMON_KEY_SHA256


def key_from_arg(arg):
    if re.fullmatch(r'[0-9a-fA-F]{32}', arg):
        return bytes.fromhex(arg)
    data = open(arg, 'rb').read()
    if len(data) == 16:
        return data
    if len(data) >= 0x124:  # BootMii keys.bin
        return data[0x114:0x124]
    return None


def key_from_dolphin(exe_path):
    """Find the key in Dolphin.exe, where the compiler stores it as four
    `mov dword [rbp+disp], imm32` instructions. Verified by hash."""
    data = open(exe_path, 'rb').read()
    pattern = re.compile(rb'\xc7\x45(.)(....)', re.S)
    for m in pattern.finditer(data):
        window = data[m.start():m.start() + 96]
        words = {}
        for w in pattern.finditer(window):
            words.setdefault(w.group(1)[0], w.group(2))
        d = m.group(1)[0]
        parts = [words.get((d + 4 * i) & 0xff) for i in range(4)]
        if all(parts):
            key = b''.join(parts)
            if check_key(key):
                return key
    return None


# --- Boot stub ------------------------------------------------------------

def assemble_bootstub(devkitpro):
    src = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'channel_bootstub.S')
    gcc = os.path.join(devkitpro, 'devkitPPC', 'bin', 'powerpc-eabi-gcc')
    with tempfile.TemporaryDirectory() as tmp:
        obj = os.path.join(tmp, 'stub.o')
        subprocess.run([gcc, '-c', '-o', obj, src], check=True)
        from elftools.elf.elffile import ELFFile
        with open(obj, 'rb') as f:
            return ELFFile(f).get_section_by_name('.text').data()


def build_menu(devkitpro, boot_dol_path, root, keep_elf=None):
    """Build channel/ (libogc menu app) with boot.dol embedded, and return the DOL."""
    gcc = os.path.join(devkitpro, 'devkitPPC', 'bin', 'powerpc-eabi-gcc')
    ogc = os.path.join(devkitpro, 'libogc')
    arch = ['-mrvl', '-mcpu=750', '-meabi', '-mhard-float']
    src = os.path.join(root, 'channel')
    with tempfile.TemporaryDirectory() as tmp:
        incbin = os.path.join(tmp, 'bootdol.S')
        with open(incbin, 'w') as f:
            boot = os.path.abspath(boot_dol_path).replace(os.sep, '/')
            f.write('.section .rodata\n.balign 32\n.global boot_dol\nboot_dol:\n'
                    f'.incbin "{boot}"\n.global boot_dol_end\nboot_dol_end:\n')
        elf = os.path.join(tmp, 'menu.elf')
        subprocess.run([gcc, *arch, '-O2', '-Wall', '-DGEKKO', '-I', os.path.join(ogc, 'include'),
                        os.path.join(src, 'main.c'), os.path.join(src, 'relocator.S'), incbin,
                        '-L', os.path.join(ogc, 'lib', 'wii'), '-lwiiuse', '-lbte', '-logc', '-lm',
                        '-o', elf], check=True)
        if keep_elf:
            shutil.copy(elf, keep_elf)
        dol = os.path.join(tmp, 'menu.dol')
        subprocess.run([sys.executable, os.path.join(root, 'elf2dol.py'), elf, dol], check=True)
        return open(dol, 'rb').read()


def add_bootstub(dol, stub):
    """Put the stub in a free text section at 0x80003400, pointing at the DOL's entry."""
    hdr = list(struct.unpack('>64I', dol[:0x100]))
    entry = hdr[56]
    stub = stub[:-4] + struct.pack('>I', entry)
    stub = pad(stub, 0x20)
    for i in range(7):  # text sections
        if hdr[36 + i] == 0:
            break
    else:
        sys.exit('no free text section in the DOL')
    offset = align(len(dol), 0x20)
    hdr[i], hdr[18 + i], hdr[36 + i] = offset, 0x80003400, len(stub)
    return struct.pack('>64I', *hdr) + dol[0x100:] + b'\0' * (offset - len(dol)) + stub


# --- Banner ---------------------------------------------------------------

def extract_banner(dolphin_tool, disc):
    with tempfile.TemporaryDirectory() as tmp:
        subprocess.run([dolphin_tool, 'extract', '-i', disc, '-o', tmp, '-g', '-s', 'opening.bnr',
                        '-q'], check=True)
        for root, _, files in os.walk(tmp):
            if 'opening.bnr' in files:
                return open(os.path.join(root, 'opening.bnr'), 'rb').read()
    sys.exit(f'opening.bnr not found in {disc}')


def rename_banner(bnr, name):
    """Set the channel name shown in the System Menu and fix the IMET hash."""
    bnr = bytearray(bnr)
    if bnr[0x40:0x44] != b'IMET':
        sys.exit('opening.bnr has no IMET header')
    encoded = name.encode('utf-16-be')[:0x52]
    for lang in range(10):
        off = 0x5C + lang * 0x54
        bnr[off:off + 0x54] = encoded + b'\0' * (0x54 - len(encoded))
    bnr[0x5F0:0x600] = b'\0' * 16
    bnr[0x5F0:0x600] = hashlib.md5(bnr[0:0x600]).digest()
    return bytes(bnr)


# --- Ticket / TMD -----------------------------------------------------------

def fakesign(blob, field):
    """Zero the signature and brute-force a spare u16 so SHA-1 starts with 00."""
    blob = bytearray(blob)
    blob[4:0x104] = b'\0' * 0x100
    for i in range(0x10000):
        struct.pack_into('>H', blob, field, i)
        if hashlib.sha1(blob[0x140:]).digest()[0] == 0:
            return bytes(blob)
    sys.exit('fakesigning failed')


def make_ticket(enc_title_key):
    t = bytearray(0x2A4)
    struct.pack_into('>I', t, 0, 0x10001)
    t[0x140:0x140 + 26] = b'Root-CA00000001-XS00000003'
    t[0x1BF:0x1CF] = enc_title_key
    struct.pack_into('>Q', t, 0x1D0, 0)            # ticket id
    struct.pack_into('>Q', t, 0x1DC, TITLE_ID)
    struct.pack_into('>H', t, 0x1E4, 0xFFFF)
    struct.pack_into('>I', t, 0x1E8, 0xFFFFFFFF)   # permitted titles mask
    t[0x1F1] = 0                                    # common key index
    t[0x222:0x262] = b'\xff' * 0x40                # content access
    return fakesign(t, 0x262)


def make_tmd(contents, boot_index, version):
    t = bytearray(0x1E4 + 36 * len(contents))
    struct.pack_into('>I', t, 0, 0x10001)
    t[0x140:0x140 + 26] = b'Root-CA00000001-CP00000004'
    struct.pack_into('>Q', t, 0x184, IOS_ID)
    struct.pack_into('>Q', t, 0x18C, TITLE_ID)
    struct.pack_into('>I', t, 0x194, 1)
    struct.pack_into('>H', t, 0x198, 0x3031)       # group "01"
    struct.pack_into('>H', t, 0x19C, 3)            # region free
    t[0x1B2] = 1                                    # skip drive reset, as mkw-sp does
    struct.pack_into('>H', t, 0x1DC, version)
    struct.pack_into('>H', t, 0x1DE, len(contents))
    struct.pack_into('>H', t, 0x1E0, boot_index)
    for i, data in enumerate(contents):
        struct.pack_into('>IHHQ', t, 0x1E4 + 36 * i, i, i, 1, len(data))
        t[0x1E4 + 36 * i + 16:0x1E4 + 36 * i + 36] = hashlib.sha1(data).digest()
    return fakesign(t, 0x1E2)


def build_wad(cert_chain, ticket, tmd, data):
    header = struct.pack('>IHHIIIIII', 0x20, 0x4973, 0, len(cert_chain), 0, len(ticket), len(tmd),
                         len(data), 0)
    return b''.join(pad(x) for x in (header, cert_chain, ticket, tmd, data))


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dolphin_dir = r'C:\Users\you\Desktop\Games\Dolphin-x64'
    appdata = os.environ.get('APPDATA', '')
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    p.add_argument('--profile', default='debug', help='build profile under out/ (default: debug)')
    p.add_argument('--dol', help='DOL to wrap (default: out/<profile>/boot.dol)')
    p.add_argument('--disc', default=os.path.join(dolphin_dir, 'Mario Kart Wii (USA) (En,Fr,Es).wbfs'),
                   help='your MKW disc image, for the banner')
    p.add_argument('--dolphin-tool', default=os.path.join(dolphin_dir, 'DolphinTool.exe'))
    p.add_argument('--dolphin-exe', default=os.path.join(dolphin_dir, 'Dolphin.exe'),
                   help='Dolphin.exe to read the common key from')
    p.add_argument('--devkitpro', default=os.environ.get('DEVKITPRO'),
                   help='devkitPro folder with devkitPPC and libogc, for the menu app '
                        '(default: $DEVKITPRO)')
    p.add_argument('--common-key', help='key file, hex string, or BootMii keys.bin')
    p.add_argument('--cert-chain', default=os.path.join(appdata, 'Dolphin Emulator', 'Wii', 'sys',
                                                        'cert.sys'),
                   help='CA/CP/XS certificate chain (default: Dolphin NAND cert.sys)')
    p.add_argument('--name', default=CHANNEL_NAME, help='channel name in the System Menu')
    p.add_argument('--version', type=int, default=1, help='title version')
    p.add_argument('-o', '--output', help='output .wad (default: out/channel/mkw-selfhost.wad)')
    args = p.parse_args()

    dol_path = args.dol or os.path.join(root, 'out', args.profile, 'boot.dol')
    out_path = args.output or os.path.join(root, 'out', 'channel', 'mkw-selfhost.wad')

    key = key_from_arg(args.common_key) if args.common_key else key_from_dolphin(args.dolphin_exe)
    if not check_key(key):
        sys.exit('Could not get the Wii common key. Pass --common-key (file, hex, or keys.bin).')

    cert_chain = open(args.cert_chain, 'rb').read()
    if not args.devkitpro:
        sys.exit('Set DEVKITPRO or pass --devkitpro (needs devkitPPC and libogc).')
    menu = build_menu(args.devkitpro, dol_path, root)
    dol = add_bootstub(menu, assemble_bootstub(args.devkitpro))
    banner = rename_banner(extract_banner(args.dolphin_tool, args.disc), args.name)

    # Content 0: banner. Content 1: the menu app (with boot.dol inside) plus the boot
    # stub, run directly by IOS.
    contents = [banner, dol]
    title_key = hashlib.sha1(b'mkw-selfhost-channel').digest()[:16]
    title_iv = struct.pack('>Q', TITLE_ID) + b'\0' * 8
    enc_title_key = aes_cbc_encrypt(key, title_iv, title_key)

    data = b''
    for i, c in enumerate(contents):
        iv = struct.pack('>H', i) + b'\0' * 14
        data += pad(aes_cbc_encrypt(title_key, iv, pad(c, 16)))

    wad = build_wad(cert_chain, make_ticket(enc_title_key), make_tmd(contents, 1, args.version),
                    data)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(wad)
    print(f'Wrote {out_path} ({len(wad)} bytes), title 00010001-MKSH')


if __name__ == '__main__':
    main()
