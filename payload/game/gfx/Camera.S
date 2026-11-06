// See LICENSE-mkw-sp

#include <Common.S>

// 16:9 FOV setting: with "Wider", use the 4:3 camera parameters (0x10 bytes earlier) in 16:9.
PATCH_B_START(Camera_805a2034, 0xbc)
    mr r29, r3
    bl Settings_GetFOV169
    slwi r3, r3, 4
    subf r3, r3, r29
    addi r0, r3, 0x10

    b Camera_805a2034 + 0xc0
PATCH_B_END(Camera_805a2034, 0xbc)
