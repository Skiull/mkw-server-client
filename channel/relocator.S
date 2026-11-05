# Loads a DOL into place and jumps to it. main.c copies this to 0x80001800 and
# calls it with r3 = the DOL, after shutting everything down. Position
# independent and uses no stack, since loading the DOL may overwrite the menu.

    .section .text
    .global relocator
    .global relocator_end
relocator:
    li      4, 0                # section index (7 text + 11 data)
1:  slwi    5, 4, 2
    lwzx    6, 3, 5             # file offset
    addi    7, 5, 0x48
    lwzx    7, 3, 7             # load address
    addi    8, 5, 0x90
    lwzx    8, 3, 8             # size
    cmpwi   8, 0
    beq     3f
    add     6, 6, 3
    srwi    9, 8, 2
    mtctr   9
    subi    6, 6, 4
    subi    10, 7, 4
2:  lwzu    11, 4(6)
    stwu    11, 4(10)
    bdnz    2b
    bl      flush
3:  addi    4, 4, 1
    cmpwi   4, 18
    blt     1b

    # Clear the BSS
    lwz     7, 0xd8(3)
    lwz     8, 0xdc(3)
    cmpwi   8, 0
    beq     5f
    srwi    9, 8, 2
    mtctr   9
    subi    10, 7, 4
    li      11, 0
4:  stwu    11, 4(10)
    bdnz    4b
    bl      flush

5:  sync
    isync
    lwz     6, 0xe0(3)          # entry point
    mtctr   6
    bctr

# Write back and invalidate r8 bytes at r7 (clobbers r9, r10, ctr)
flush:
    addi    9, 8, 31
    srwi    9, 9, 5
    mtctr   9
    mr      10, 7
6:  dcbst   0, 10
    sync
    icbi    0, 10
    addi    10, 10, 32
    bdnz    6b
    sync
    isync
    blr
relocator_end:
