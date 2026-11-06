#pragma once

#include "game/ui/UIControl.hh"

namespace UI {

// The name tag over a kart in races.
class CtrlRaceNameBalloon : public LayoutUIControl {
public:
    // Shows the player's MKW Lounge rank instead of the star rank, if they chose to.
    REPLACE void refresh(u32 playerId);
    void REPLACED(refresh)(u32 playerId);
};

} // namespace UI
