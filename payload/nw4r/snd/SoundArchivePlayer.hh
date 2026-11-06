// See LICENSE-mkw-sp

#pragma once

#include "nw4r/snd/SoundPlayer.hh"

namespace nw4r::snd {

class SoundArchivePlayer {
public:
    void REPLACED(update)();
    REPLACE void update();

private:
    u8 _00[0x30 - 0x00];
    u32 m_playerCount;
    SoundPlayer *m_players;
    u8 _38[0xe0 - 0x38];
};
static_assert(sizeof(SoundArchivePlayer) == 0xe0);

} // namespace nw4r::snd
