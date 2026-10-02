#pragma once

#include <Common.hh>

extern "C" {
#include <revolution/os.h>
}

#include <game/util/Registry.hh>

namespace Net {

struct RaceInfoRecord {
    // Starts when countdown begins. See RaceManager.m_countdownTimer
    u32 timeSinceCountdown;

    // Random seed. Non-zero for racers and zero for spectators and players in globe scene
    // Its both used for rng and deciding who's racing
    u32 raceSeed;

    // When battle, true if coin runners, false if balloon battle.
    // When VS, true if team VS, false if solo VS.
    bool isCoinRunnersOrTeamVS : 1;
    u32 _8_1 : 19;
    // Indexed by playerId to indicate teams. PlayerId slot is 1 if red team. Cleared if solo VS.
    u32 playerIdTeamMap : 12;

    u16 lagFrames;
    Registry::Vehicle p1Vehicle : 8;
    Registry::Vehicle p2Vehicle : 8;
    Registry::Character p1Character : 8;
    Registry::Character p2Character : 8;
    u16 introCameraTimeAdjLatency;
    u16 starRank;
    Registry::Course coursePlayed : 8;
    // sets bitfield when != 0, but not sure how it gets set.
    // wiki refers to this as playerType, but ghidra doesn't pick up on RaceInfoRecord well
    // automatically so can't corroborated yet
    u8 _17;
    u8 aidMap[12];
    Registry::EngineClass engineClass : 8;
    bool startedCountdown : 8; // added, was padding.
    u8 _26[0x28 - 0x26];
};
static_assert(sizeof(RaceInfoRecord) == 0x28);

struct RaceInfoPlayer {
    u32 timer;
    Registry::Vehicle player1Vehicle;
    Registry::Vehicle player2Vehicle;
    Registry::Character player1Character;
    Registry::Character player2Character;
    Registry::Course course;
    u8 _18[0x20 - 0x18];
    u8 playerIdToAidMapping[12];
    u8 _2c[0x30 - 0x2c];
};
static_assert(sizeof(RaceInfoPlayer) == 0x30);

} // namespace Net
