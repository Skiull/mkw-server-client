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

    struct Player {
        u8 _00[0x08 - 0x00];
        Registry::Vehicle vehicle;
        Registry::Character character;
        u8 _10[0xcc - 0x10];
        Registry::Team team;
        u8 _d0[0xd8 - 0xd0];
        u16 points;
        u8 _da[0xe1 - 0xda];
        u8 startingSpot;
        u8 _e2[0xf0 - 0xe2];
    };
    static_assert(sizeof(Player) == 0xf0);

    struct Scenario {
        bool isOnline() const;
        bool isOnlineVS() const;
        bool isOnlineBattle() const;
        u8 _000[0x008 - 0x000];
        Player players[12];
        Registry::Course course;
        Registry::EngineClass engineClass;
        Registry::GameMode gameMode;
        u8 _b54[0xb58 - 0xb54];
        Registry::BattleType battleType;
        u8 _b5c[0xb73 - 0xb5c];
        Registry::ModeFlags modeFlags : 8;
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
