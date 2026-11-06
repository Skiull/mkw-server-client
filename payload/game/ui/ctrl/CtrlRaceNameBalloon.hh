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

// Picks which players get a name tag. Ported from mkw-sp (see LICENSE-mkw-sp) for the Player
// Tags setting: up to 12 tags instead of vanilla's few (CtrlRaceNameBalloon.S grows the arrays).
class BalloonManager {
public:
    REPLACE BalloonManager();
    ~BalloonManager();
    REPLACE void dt(s32 type);

    REPLACE void init(u8 localPlayerId);
    REPLACE void addNameControl(CtrlRaceNameBalloon *nameControl);
    void REPLACED(calc)();
    REPLACE void calc();

private:
    u8 _00[0x03 - 0x00];
    u8 m_localPlayerId;
    u32 m_nameCount = 0;
    u8 _08[0x14 - 0x08];
    s32 m_playerIds[12];
    bool m_nameIsEnabled[12]; // Moved
    Vec3 *m_namePositions;    // Modified: 12 of them, allocated in the constructor
    u8 _54[0xc4 - 0x54];
};
static_assert(sizeof(BalloonManager) == 0xc4);

} // namespace UI
