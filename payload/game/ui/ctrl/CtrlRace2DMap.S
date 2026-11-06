// See LICENSE-mkw-sp

#include <Common.S>

// Map Icons setting: with "Miis", local players (outside split screen) get their Mii as their
// minimap icon instead of their character.
PATCH_BL_START(CtrlRace2DMapCharacter_initSelf, 0x294)
    lwz r4, 0x20 + 0x8 + 0x10 (r3) // Get the player type

    lwz r3, 0x20 + 0x8 + 0xc (r3) // Get the character id

    // Check that the character id is not already a Mii
    cmpwi r3, 0x18
    bgelr

    // Check that the player type is not cpu
    cmpwi r4, 0x1
    beqlr

    // Check that we are not in split-screen mode
    lis r4, s_raceConfig@ha
    lwz r4, s_raceConfig@l (r4)
    lbz r4, 0x20 + 0x6 (r4)
    cmpwi r4, 0x1
    bgtlr

    mflr r29
    mr r28, r3

    bl Settings_GetMapIcons
    cmpwi r3, 0x1

    mr r3, r28
    mtlr r29

    // Check that the setting is enabled
    bnelr

    // Set the id to a Mii one (it's only used for the comparison)
    li r3, 0x18
    b CtrlRace2DMapCharacter_initSelf + 0x298
PATCH_BL_END(CtrlRace2DMapCharacter_initSelf, 0x294)
