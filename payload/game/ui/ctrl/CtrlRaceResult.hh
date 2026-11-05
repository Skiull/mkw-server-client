#pragma once

#include "game/ui/UIControl.hh"

namespace UI {

// A row of the race results screen.
class CtrlRaceResult : public LayoutUIControl {
public:
    // Also shows the player's MKW Lounge rank instead of the star rank, if they chose to.
    REPLACE void refreshRankIconsColor(u32 rank, u32 playerId);
    void REPLACED(refreshRankIconsColor)(u32 rank, u32 playerId);
};

} // namespace UI
