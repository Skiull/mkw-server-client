#pragma once

#include <Common.hh>

#include <game/net/records/User.hh>
#include <game/system/InputManager.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>
#include <game/ui/SectionManager.hh>
#include <game/util/Registry.hh>

extern "C" {
#include <revolution.h>
}

namespace MKWServer {

static constexpr u16 ConfigMagic = 0x4346;

struct __attribute__((packed)) Race {
    Race(const System::RaceConfig::Scenario &scenario)
        : course(scenario.course), engineClass(scenario.engineClass), gameMode(scenario.gameMode),
          battleType(scenario.battleType), modeFlags(scenario.modeFlags) {}
    Registry::Course course : 8;
    Registry::EngineClass engineClass : 8;
    Registry::GameMode gameMode : 8;
    Registry::BattleType battleType : 8;
    Registry::ModeFlags modeFlags : 8;
};
static_assert(sizeof(Race) == 0x5);

struct __attribute__((packed)) Aid {
    Aid(const Net::UserRecord *user) : regionLine(user->regionLine), rflWifiData(user->wifiData) {}

    Registry::RegionLine regionLine : 8;
    RFLWifiData rflWifiData;
};
static_assert(sizeof(Aid) == 0xa1);

struct __attribute__((packed)) Player {
    Player() = default;
    Player(u8 aid, u8 playerId, u8 localPlayerId, const System::RaceConfig::Player &player)
        : aid(aid), playerId(playerId), character(player.character), vehicle(player.vehicle),
          team(player.team), startingSpot(player.startingSpot),
          driftType(System::RaceManager::Instance()->player(playerId)->padProxy()->driftType()),
          rankIconId(UI::SectionManager::Instance()->sectionParams()->getRankIconId(localPlayerId)),
          points(player.points), rating(Net::UserHandler::Instance()->getRating()) {}

    u8 aid;
    u8 playerId;
    Registry::Character character : 8;
    Registry::Vehicle vehicle : 8;
    Registry::Team team : 8;
    u8 startingSpot;
    Registry::DriftType driftType : 8;
    u8 rankIconId;
    u16 points;
    u16 rating;
};
static_assert(sizeof(Player) == 0xc);

struct __attribute__((packed)) ConfigPacket {
    ConfigPacket(const Race &race, const Aid &aid) : race(race), aid(aid), players{} {}

    static s32 CalcPacketSize(u8 localPlayerCount) {
        return localPlayerCount == 2 ? sizeof(ConfigPacket) : sizeof(ConfigPacket) - sizeof(Player);
    }

    u16 magic = ConfigMagic;
    Race race;
    Aid aid;
    Player players[2];
};
static_assert(sizeof(ConfigPacket) == 0xc0);

} // namespace MKWServer
