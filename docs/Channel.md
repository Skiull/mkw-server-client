# Wii channel (.wad)

`tools/make_wad.py` turns `out/<profile>/boot.dol` into an installable channel,
`out/channel/mkw-selfhost.wad` (title `00010001-MKSH`, shown as "MKW Self-host").
Starting the channel boots Mario Kart Wii from the disc with the payload, the same as
launching `boot.dol` from the Homebrew Channel. For now there is no menu: the channel
goes straight to "Launch Game".

## Building

Build the client first (`build.py`), then:

```bash
PYTHONPATH=<pylib with cryptography + pyelftools> DEVKITPPC=<devkitPPC> python tools/make_wad.py
```

Useful options: `--profile release`, `--disc <your MKW .wbfs/.iso>`, `--common-key <file|hex|keys.bin>`,
`--name "..."`, `--version N`, `-o out.wad`. Run with `--help` for the rest.

No Nintendo files are kept in the repo. At build time the script reads:

- the banner (`opening.bnr`) from your own disc image, through `DolphinTool.exe extract`
- the certificate chain from Dolphin's NAND (`%APPDATA%\Dolphin Emulator\Wii\sys\cert.sys`)
- the Wii common key from your `Dolphin.exe`, or from `--common-key` (for example a BootMii `keys.bin`)

## How it boots

IOS starts a channel's main DOL in real mode at address 0x3400, where Nintendo's DOLs keep
their startup code. `boot.dol` has nothing there, so the script adds a small stub
(`tools/channel_bootstub.S`) as a DOL section at 0x80003400. The stub sets up the memory
mappings, installs the system call handler, fills in the low-memory values a disc boot
normally sets (clock speeds, memory size), turns on the caches and jumps to `boot.dol`'s
normal entry point. The TMD asks for IOS58, which the loader needs.

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

Not yet tested on real hardware: the boot stub was only verified in Dolphin.
