// See LICENSE-mkw-sp

#include <Common.S>

// Player Tags setting: if the number of CtrlRaceNameBalloon is not zero, set it to 12
// (RacePage::getControlCount makes room for them).

PATCH_REPLACE_START(RacePage_initControls, 0x5cc)
    li r3, 12
PATCH_REPLACE_END(RacePage_initControls, 0x5cc)

PATCH_REPLACE_START(RacePage_initControls, 0x680)
    li r3, 12
PATCH_REPLACE_END(RacePage_initControls, 0x680)
