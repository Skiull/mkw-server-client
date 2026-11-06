// See LICENSE-mkw-sp

#pragma once

#include <Common.hh>

namespace Sound {

class RaceSoundManager {
public:
    u32 state() const {
        return m_state;
    }

    void transition(u32 state);

    static RaceSoundManager *Instance() {
        return s_instance;
    }

private:
    u8 _000[0x040 - 0x000];
    u32 m_state;
    u8 _044[0x178 - 0x044];

    static RaceSoundManager *s_instance;
};

} // namespace Sound
