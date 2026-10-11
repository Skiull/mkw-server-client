#pragma once

#include <Common.hh>

namespace UI {

struct Player {
    u8 _0[0x8 - 0x0];
    u8 rankIconId;
    u8 _9[0xc - 0x9];
};
static_assert(sizeof(Player) == 0xc);

class SectionParams {
public:
    u8 getLocalPlayerCount() const;

    u8 getRankIconId(u8 localPlayerId) const {
        return m_players[localPlayerId].rankIconId;
    }

private:
    u8 _000[0x124 - 0x000];
    u32 m_localPlayerCount;
    u8 _128[0x220 - 0x128];
    Player m_players[2];
    u8 _238[0x510 - 0x238];
};
static_assert(sizeof(SectionParams) == 0x510);

} // namespace UI
