// See LICENSE-mkw-sp

#pragma once

#include "game/ui/ctrl/CtrlRaceBase.hh"

namespace UI {

class CtrlRace2DMap : public CtrlRaceBase {
public:
    CtrlRace2DMap();
    ~CtrlRace2DMap() override;

    void REPLACED(calcSelf)();
    REPLACE void calcSelf() override;

private:
    u8 _198[0x1a4 - 0x198];
};
static_assert(sizeof(CtrlRace2DMap) == 0x1a4);

} // namespace UI
