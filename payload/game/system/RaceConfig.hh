#pragma once

#include <Common.hh>

#include "game/util/Registry.hh"

namespace System {

/// Configuration for the current race.
///
/// For configuration over a set of VS/BT races, see GlobalContext.
class RaceConfig {
public:
    RaceConfig();
    virtual ~RaceConfig();
    virtual void dt(s32 type);

    enum class GameMode {
        OfflineVS = 1,
        TimeAttack = 2,
        OfflineBT = 3,
        Mission = 4,
        OnlinePrivateVS = 7,
        OnlinePrivateBT = 10,
        Awards = 11,
    };

    struct Player {
        enum class Type {
            Local = 0,
            CPU = 1,
            Ghost = 3,
            Online = 4,
            None = 5,
        };

        u8 _00[0x05 - 0x00];
        s8 screenId;
        s8 ghostProxyId;
        u8 _07[0x08 - 0x07];
        u32 vehicleId;
        u32 characterId;
        Type type;
        u8 _14[0xf0 - 0x14];
    };
    static_assert(sizeof(Player) == 0xf0);

    struct Scenario {
        bool mirror() const {
            return modeFlags & 1;
        }

        u8 _000[0x004 - 0x000];
        u8 playerCount;
        u8 screenCount;
        u8 localPlayerCount;
        u8 _007[0x008 - 0x007];
        Player players[12];
        u8 _b48[0xb50 - 0xb48];
        GameMode gameMode;
        u8 _b54[0xb70 - 0xb54];
        u32 modeFlags; // 1: mirror, 2: teams, 4: competition
        u8 _b74[0xbf0 - 0xb74];
    };
    static_assert(sizeof(Scenario) == 0xbf0);

    Scenario &raceScenario();

    static RaceConfig *Instance();

private:
    u8 _0004[0x0020 - 0x0004];

public:
    Scenario m_raceScenario;
    u8 _0c10[0x73f0 - 0x0c10];

    static RaceConfig *s_instance;
};
static_assert(sizeof(RaceConfig) == 0x73f0);

} // namespace System
