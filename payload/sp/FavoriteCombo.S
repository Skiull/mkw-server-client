#include <Common.S>

// Favorite Combo: Bully, CLF78 and Kazuki's "Set User Favorite Combo as Default like CTGP"
// code (https://mariokartwii.com/showthread.php?tid=2089) as a setting. When the menu
// context is reset, the game sets each player's vehicle to -1 and player 1's character to
// Mario; these hooks use the license's most-played character and vehicle instead
// (sp/FavoriteCombo.cc). r10/r3 is the context. Each hook saves the volatile registers and
// ctr the loop uses.


PATCH_B_START(FavCombo_805e4114, 0x0)
    stwu r1, -0x30 (r1)
    mflr r0
    stw r0, 0x34 (r1)
    stw r3, 0x8 (r1)
    stw r4, 0xc (r1)
    stw r5, 0x10 (r1)
    stw r6, 0x14 (r1)
    stw r7, 0x18 (r1)
    stw r8, 0x1c (r1)
    stw r9, 0x20 (r1)
    stw r10, 0x24 (r1)
    stw r11, 0x28 (r1)
    mfctr r0 // The loop counter (bdnz below)
    stw r0, 0x2c (r1)
    mr r3, r9 // Default (-1)
    bl FavoriteCombo_vehicle
    mr r12, r3
    lwz r3, 0x8 (r1)
    lwz r4, 0xc (r1)
    lwz r5, 0x10 (r1)
    lwz r6, 0x14 (r1)
    lwz r7, 0x18 (r1)
    lwz r8, 0x1c (r1)
    lwz r9, 0x20 (r1)
    lwz r10, 0x24 (r1)
    lwz r11, 0x28 (r1)
    lwz r0, 0x2c (r1)
    mtctr r0
    lwz r0, 0x34 (r1)
    mtlr r0
    addi r1, r1, 0x30
    stw r12, 316 (r10) // Original: stw r9, 316 (r10)
    b FavCombo_805e4114 + 0x4
PATCH_B_END(FavCombo_805e4114, 0x0)

PATCH_B_START(FavCombo_805e418c, 0x0)
    stwu r1, -0x30 (r1)
    mflr r0
    stw r0, 0x34 (r1)
    stw r3, 0x8 (r1)
    stw r4, 0xc (r1)
    stw r5, 0x10 (r1)
    stw r6, 0x14 (r1)
    stw r7, 0x18 (r1)
    stw r8, 0x1c (r1)
    stw r9, 0x20 (r1)
    stw r10, 0x24 (r1)
    stw r11, 0x28 (r1)
    mfctr r0 // The loop counter (bdnz below)
    stw r0, 0x2c (r1)
    mr r3, r9 // Default (-1)
    bl FavoriteCombo_vehicle
    mr r12, r3
    lwz r3, 0x8 (r1)
    lwz r4, 0xc (r1)
    lwz r5, 0x10 (r1)
    lwz r6, 0x14 (r1)
    lwz r7, 0x18 (r1)
    lwz r8, 0x1c (r1)
    lwz r9, 0x20 (r1)
    lwz r10, 0x24 (r1)
    lwz r11, 0x28 (r1)
    lwz r0, 0x2c (r1)
    mtctr r0
    lwz r0, 0x34 (r1)
    mtlr r0
    addi r1, r1, 0x30
    stw r12, 320 (r10) // Original: stw r9, 320 (r10)
    b FavCombo_805e418c + 0x4
PATCH_B_END(FavCombo_805e418c, 0x0)


PATCH_B_START(FavCombo_805e4208, 0x0)
    stwu r1, -0x10 (r1)
    mflr r0
    stw r0, 0x14 (r1)
    stw r3, 0x8 (r1)
    bl FavoriteCombo_character
    mr r5, r3 // Original: li r5, 0
    lwz r3, 0x8 (r1)
    lwz r0, 0x14 (r1)
    mtlr r0
    addi r1, r1, 0x10
    b FavCombo_805e4208 + 0x4
PATCH_B_END(FavCombo_805e4208, 0x0)
