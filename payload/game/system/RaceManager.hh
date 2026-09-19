#pragma once

#include <Common.hh>

#include "game/system/InputManager.hh"

namespace System {

class RaceManager {
public:
    enum class RaceState {
        IntroCamera = 0,
        Countdown = 1,
        Racing = 2,
        Finished = 3,
        FinishedTimeAttack = 4,
    };

    class Player {
    public:
        const PadProxy *padProxy() const {
            return m_padProxy;
        }

    private:
        REPLACE void calc();
        void REPLACED(calc)();

        u8 _00[0x08 - 0x00];
        u8 m_playerId;
        u8 _09[0x48 - 0x09];
        PadProxy *m_padProxy;
        u8 _4c[0x54 - 0x4c];
    };
    static_assert(sizeof(Player) == 0x54);

    const Player *player(u32 playerId) const {
        return m_players[playerId];
    }

    u16 introTimer() const {
        return m_introTimer;
    }

    u32 countdownTimer() const {
        return m_countdownTimer;
    }

    void startCountdown() {
        m_canCountdownStart = true;
    }

    bool isRaceOngoing() const;

    static RaceManager *Instance() {
        return s_instance;
    }

private:
    REPLACE void update();
    void REPLACED(update)();

    u8 _00[0x0c - 0x00];
    Player **m_players;
    u8 _10[0x1c - 0x10];
    u8 m_finishedPlayerCount;
    u8 m_disconnectedPlayerCount;

    // Begins counting down when loaded into race, stops when countdown begins
    u16 m_introTimer;

    // Begins counting when race countdown starts
    u32 m_countdownTimer;
    u8 _24[0x28 - 0x24];
    RaceState m_state;
    bool m_introWasSkipped;
    bool m_spectatorMode;
    bool m_canCountdownStart;
    bool m_cutSceneMode;
    bool m_lapCountingIsEnabled;
    u8 _31[0x4c - 0x31];

    static RaceManager *s_instance;
};
static_assert(sizeof(RaceManager) == 0x4c);

} // namespace System
