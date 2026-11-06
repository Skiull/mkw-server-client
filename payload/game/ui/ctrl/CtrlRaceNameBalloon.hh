#pragma once

#include "game/ui/UIControl.hh"

namespace UI {

// The name tag over a kart in races.
class CtrlRaceNameBalloon : public LayoutUIControl {
public:
    // Shows the player's MKW Lounge rank instead of the star rank, if they chose to.
    REPLACE void refresh(u32 playerId);
    void REPLACED(refresh)(u32 playerId);
    // Applies the Time Trials ghost tag visibility setting.
    void REPLACED(calcSelf)();
    REPLACE void calcSelf() override;

private:
    void refreshGhostText(u32 playerId);

    u8 _174[0x178 - 0x174];
    s32 m_playerId;
    u8 _17c[0x188 - 0x17c];
};
static_assert(sizeof(CtrlRaceNameBalloon) == 0x188);

} // namespace UI
