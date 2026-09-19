#pragma once

#include <Common.hh>

extern "C" {
#include <revolution.h>
}

#include <sp/net/mkw_server/RaceManager.hh>
#include <sp/net/mkw_server/packets/PingTime.hh>
#include <sp/net/mkw_server/packets/SearchId.hh>

namespace MKWServer {

enum class SearchRegion : s8 {
    None = -1,
    Private = 0,
    WW = 1,
    NA = 2,
    EU = 3,
    JP = 4,
    KOR = 5,
};

// different race packet types, currently we check the first byte (which should be reconsidered at
// some point) for the type of packet
enum class PacketType : s8 {
    RacePacket = 0xB,
};

void setMKWServerAddress(u32 addr, u16 port);

void resetMKWServerInfo();

bool applyMKWServerHeader(void *packet, u8 myAid);

u64 getSearchId();

bool hasMKWServerAddress();

bool sendRaceFrame(const RaceFramePacket *raceFrame);

void sendReadyPacket();

PingTime getPing();

bool trySendRacePacketToMKWServer(const void *data, u32 size);

bool verifySearchIdMagic(const u8 *packet, u32 size);

bool handleSearchIdPacket(const u8 *packet, u32 size);

void sendPong();

void setPingTime(const u8 *message);

bool sendMessageToQR2(const u8 *data, u32 size);
} // namespace MKWServer
