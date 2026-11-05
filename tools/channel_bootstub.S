# Channel boot stub, added to boot.dol by make_wad.py as a section at 0x80003400.
#
# When the System Menu launches a channel, IOS loads the boot DOL and starts the
# PPC at physical 0x3400 in real mode (Nintendo DOLs carry SDK init code there).
# This stub maps memory with BATs like the SDK does, turns on address translation
# and jumps to the DOL's normal entry point (patched into the last word).

    .section .text
    .global _start
_start:
    li      0, 0
    # Enable BATs 4-7 (HID4.SBE); the SDK keeps MEM2 mappings there
    mfspr   3, 1011     # HID4
    oris    3, 3, 0x0200
    mtspr   1011, 3
    isync

    # Clear all BATs first
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

    # Invalidate and enable the L1 caches, like the SDK's init code does
    mfspr   3, 1008     # HID0
    ori     3, 3, 0x0c00 # ICFI | DCFI
    mtspr   1008, 3
    isync
    ori     3, 3, 0xc000 # ICE | DCE
    sync
    mtspr   1008, 3
    isync

    # Jump to the entry point with IR, DR and FP enabled
    bl      1f
1:  mflr    3
    lwz     3, (entry - 1b)(3)
    mtsrr0  3
    mfmsr   3
    ori     3, 3, 0x2030
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

    .balign 4
entry:
    .long   0
