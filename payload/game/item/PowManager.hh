#pragma once

#include <Common.hh>

namespace Item {

// Hop dodge practice (from the henrymkw/mkw-sp fork) sets off a POW Block on demand.
class PowManager {
public:
    // The POW manager of the current race, or null. Recorded by calc(), which runs every frame,
    // so this needs no per-region address for the game's own instance pointer.
    static PowManager *Instance() {
        return s_current;
    }
    static void ClearInstance() {
        s_current = nullptr;
    }

    // Starts a POW Block used by playerIndex (who is normally not affected by it).
    void beginPow(u32 playerIndex);

    void setAffected(u32 playerIndex, bool affected) {
        if (playerIndex < 12) {
            m_playerAffected[playerIndex] = affected;
        }
    }

private:
    REPLACE void calc();
    void REPLACED(calc)();

    u32 m_powTimer;
    u8 _04[0x09 - 0x04];
    bool m_playerAffected[12];
    u8 _15[0x28 - 0x15];

    static PowManager *s_current;
};
static_assert(sizeof(PowManager) == 0x28);

} // namespace Item
