# Channel boot stub, added to boot.dol by make_wad.py as a section at 0x80003400.
#
# When the System Menu launches a channel, IOS loads the boot DOL and starts the
# PPC at physical 0x3400 in real mode (Nintendo DOLs carry SDK init code there).
# This stub resets the CPU state the way HBC's channel stub does, maps memory with
# BATs, turns on address translation and jumps to the DOL's normal entry point
# (patched into the last word).

    .section .text
    .global _start
_start:
    # Put the CPU in a known state first, the way the Homebrew Channel's own
    # channel stub does (hbc/wiipax/stub/realmode.S). On a real Wii these
    # registers hold leftovers at this point; Dolphin starts them clean.

    # HID0 = 0x00110c64: caches off and being invalidated, branch history and
    # target caches on, data cache flush assist on
    lis     3, 0x0011
    ori     3, 3, 0x0c64
    mtspr   1008, 3
    isync

    # MSR = FP on, everything else off (no translation, no interrupts)
    li      4, 0x2000
    mtmsr   4
    isync

    # Save the VI (video) registers exactly as the System Menu left them, so the
    # menu can put them back before starting boot.dol (see vi_save below).
    # Caches are off at this point, so these real-mode accesses go straight out.
    bl      9f
9:  mflr    5
    addi    5, 5, (vi_save - 9b - 2)
    lis     6, 0x0c00
    ori     6, 6, 0x2000 - 2
    li      7, 64
    mtctr   7
10: lhzu    8, 2(6)
    sthu    8, 2(5)
    bdnz    10b

    # Same for the OS globals at 0x00-0xFF (TV mode at 0xCC, clocks, memory
    # size, ...), which libogc overwrites and a disc game reads (see os_save).
    bl      11f
11: mflr    5
    addi    5, 5, (os_save - 11b - 4)
    li      6, -4
    li      7, 64
    mtctr   7
12: lwzu    8, 4(6)
    stwu    8, 4(5)
    bdnz    12b

    # Clear all BATs
    li      0, 0
    mtspr   528, 0      # IBAT0U
    mtspr   530, 0      # IBAT1U
    mtspr   532, 0      # IBAT2U
    mtspr   534, 0      # IBAT3U
    mtspr   560, 0      # IBAT4U
    mtspr   562, 0      # IBAT5U
    mtspr   564, 0      # IBAT6U
    mtspr   566, 0      # IBAT7U
    mtspr   536, 0      # DBAT0U
    mtspr   538, 0      # DBAT1U
    mtspr   540, 0      # DBAT2U
    mtspr   542, 0      # DBAT3U
    mtspr   568, 0      # DBAT4U
    mtspr   570, 0      # DBAT5U
    mtspr   572, 0      # DBAT6U
    mtspr   574, 0      # DBAT7U
    isync

    # Segment registers: 0x80000000 (direct-store, never used), so nothing
    # outside the BATs goes through stale page tables
    lis     0, 0x8000
    mtsr    0, 0
    mtsr    1, 0
    mtsr    2, 0
    mtsr    3, 0
    mtsr    4, 0
    mtsr    5, 0
    mtsr    6, 0
    mtsr    7, 0
    mtsr    8, 0
    mtsr    9, 0
    mtsr    10, 0
    mtsr    11, 0
    mtsr    12, 0
    mtsr    13, 0
    mtsr    14, 0
    mtsr    15, 0
    isync

    # 0x80000000 -> MEM1, cached (instruction + data)
    lis     3, 0x8000
    ori     3, 3, 0x1fff
    li      4, 0x0002
    mtspr   529, 4      # IBAT0L
    mtspr   528, 3      # IBAT0U
    mtspr   537, 4      # DBAT0L
    mtspr   536, 3      # DBAT0U

    # 0xC0000000 -> MEM1 + hardware registers, uncached
    lis     3, 0xc000
    ori     3, 3, 0x1fff
    li      4, 0x002a
    mtspr   539, 4      # DBAT1L
    mtspr   538, 3      # DBAT1U

    # 0x90000000 -> MEM2, cached (instruction + data)
    lis     3, 0x9000
    ori     3, 3, 0x1fff
    lis     4, 0x1000
    ori     4, 4, 0x0002
    mtspr   561, 4      # IBAT4L
    mtspr   560, 3      # IBAT4U
    mtspr   569, 4      # DBAT4L
    mtspr   568, 3      # DBAT4U

    # 0xD0000000 -> MEM2, uncached
    lis     3, 0xd000
    ori     3, 3, 0x1fff
    lis     4, 0x1000
    ori     4, 4, 0x002a
    mtspr   571, 4      # DBAT5L
    mtspr   570, 3      # DBAT5U
    isync

    # Install the SDK system call handler at 0xC00 (the apploader uses `sc`)
    bl      2f
2:  mflr    5
    addi    5, 5, (syscall - 2b - 4)
    li      6, 0x0c00 - 4
    li      7, (syscall_end - syscall) / 4
    mtctr   7
3:  lwzu    7, 4(5)
    stwu    7, 4(6)
    bdnz    3b

    # Fill in the low-memory globals a disc boot normally leaves behind (bus and
    # CPU clocks, memory size, ...). The game hangs in OSInit without them.
    # Each one is written only if it's still zero.
    bl      4f
4:  mflr    5
    addi    5, 5, (globals - 4b - 4)
    li      7, (globals_end - globals) / 8
    mtctr   7
5:  lwzu    6, 4(5)     # address
    lwzu    7, 4(5)     # value
    lwz     8, 0(6)
    cmpwi   8, 0
    bne     6f
    stw     7, 0(6)
6:  bdnz    5b
    li      7, 0x80     # application type: game (not channel)
    stb     7, 0x3184(0)

    # Caches on (they were invalidated by the HID0 write above)
    lis     3, 0x0011
    ori     3, 3, 0xcc64
    mtspr   1008, 3
    isync

    # HID4 = 0x82000000: enables BATs 4-7, which hold the MEM2 mappings
    lis     3, 0x8200
    mtspr   1011, 3
    isync

    # Jump to the entry point with IR, DR and FP enabled
    bl      1f
1:  mflr    3
    lwz     3, (entry - 1b)(3)
    mtsrr0  3
    li      3, 0x2030   # FP, IR, DR
    mtsrr1  3
    rfi

    # Copied to 0xC00: flushes the store queue by toggling HID0 (as the SDK does)
syscall:
    mfspr   9, 1008
    ori     10, 9, 0x0008
    mtspr   1008, 10
    isync
    sync
    mtspr   1008, 9
    rfi
syscall_end:

    # (address, value) pairs, same values Dolphin uses for a Wii disc boot
globals:
    .long   0x20, 0x0d15ea5e    # boot magic
    .long   0x24, 0x00000001    # boot info version
    .long   0x28, 0x01800000    # MEM1 size
    .long   0x2c, 0x00000023    # console type (retail)
    .long   0xf0, 0x01800000    # simulated memory size
    .long   0xf8, 0x0e7be2c0    # bus clock
    .long   0xfc, 0x2b73a840    # CPU clock
globals_end:

    # The System Menu's VI registers (0xCC002000-0xCC00207F), saved at boot.
    # Lives at 0x80003680; channel/main.c restores them before launching.
    .org    0x280
vi_save:
    .space  0x80

    # The System Menu's OS globals (0x80000000-0x800000FF), saved at boot.
    # Lives at 0x80003700; channel/main.c restores them before launching.
os_save:
    .space  0x100

entry:
    .long   0
