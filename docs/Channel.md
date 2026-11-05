# Wii channel (.wad)

`tools/make_wad.py` builds an installable channel, `out/channel/mkw-selfhost.wad`
(title `00010001-MKSH`, shown as "MKW Self-host"). The channel opens a menu (`channel/`)
with a **Launch Game** button (A on a Wii Remote, Classic Controller or GameCube
controller; HOME goes back to the Wii Menu). Launch Game starts the embedded
`out/<profile>/boot.dol` the way the Homebrew Channel does, which boots Mario Kart Wii
from the disc with the payload.

## Building

Build the client first (`build.py`). The menu needs devkitPro's devkitPPC **with newlib**
and **libogc** (the trimmed devkitPPC used for `build.py` has neither). Then:

```bash
DEVKITPRO=<devkitPro folder> PYTHONPATH=<pylib with cryptography + pyelftools> python tools/make_wad.py
```

`DEVKITPRO` must contain `devkitPPC/` and `libogc/`. On this machine they were unpacked
from devkitPro's official packages (devkitppc-gcc, -binutils, -newlib, -crtls, -rules and
libogc from pkg.devkitpro.org) into `C:\Users\you\Desktop\mkwserver\devkitpro` (kept out
of %TEMP%, which Windows cleans up; the downloaded packages are in its `pkgs` folder).

Useful options: `--profile release`, `--disc <your MKW .wbfs/.iso>`, `--common-key <file|hex|keys.bin>`,
`--name "..."`, `--version N`, `-o out.wad`. Run with `--help` for the rest.

No Nintendo files are kept in the repo. At build time the script reads:

- the banner (`opening.bnr`) from your own disc image, through `DolphinTool.exe extract`
- the certificate chain from Dolphin's NAND (`%APPDATA%\Dolphin Emulator\Wii\sys\cert.sys`)
- the Wii common key from your `Dolphin.exe`, or from `--common-key` (for example a BootMii `keys.bin`)

## How it boots

1. IOS starts a channel's main DOL in real mode at 0x3400, where Nintendo's DOLs keep their
   startup code. The script adds a small stub there (`tools/channel_bootstub.S`) that sets up
   memory mappings, the low-memory values a disc boot leaves behind and the caches, then
   jumps to the menu's normal entry point. The TMD asks for IOS58.
2. The menu (libogc) shows the Launch Game button.
3. On launch it copies `boot.dol` and a small relocator (`channel/relocator.S`) to MEM2,
   shuts libogc down, reloads IOS58 (like HBC), points the exception vectors at `rfi`, and
   the relocator loads `boot.dol` into place and jumps to it.

## Testing in Dolphin

1. Tools > Install WAD..., pick `out/channel/mkw-selfhost.wad`.
2. Set your MKW disc as the default ISO (right-click it > Set as Default ISO), or insert it
   with Change Disc when the loader asks.
3. Tools > Load Wii System Menu and start "MKW Self-host" (or boot the .wad directly with
   File > Open).

To remove it: Tools > Manage NAND... or reinstall over it.

## Installing on a Wii

The ticket and TMD are fakesigned, so a normal installer rejects them. You need:

- the Homebrew Channel, and IOS58 (any up-to-date Wii has it)
- a WAD manager that accepts fakesigned titles, e.g. YAWM ModMii Edition or Wii Mod Lite,
  running on an IOS with signature checks patched (a cIOS, or the app's own AHB patching)

Copy the .wad to the SD card, install it with the WAD manager, then start the channel from
the System Menu with your MKW disc inserted. As with any WAD install, have a BootMii NAND
backup first. Uninstall with the same WAD manager.

Real-hardware status: the first version (boot.dol run directly, no menu) hung at
"Escalating privileges" on a Wii. The menu version hands over a freshly reloaded IOS58 and
a libogc-initialised system, the same as HBC, which should fix that; it has only been
verified in Dolphin so far.

Adding menu items later: extend `items` / the `ITEM_*` enum in `channel/main.c` and handle
the new index next to `ITEM_LAUNCH`.
