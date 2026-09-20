#pragma once

#include <Common.hh>
#include <common/TVec3.hh>

#include "sp/CircularBuffer.hh"

namespace MKWServer {

static constexpr u8 RaceFrameMagic[2] = {'R', 'F'};

static constexpr size_t FrameBufferSize = 240;

class __attribute__((packed)) RaceFrame {
public:
    RaceFrame();

    void setRecvData(u8 aid, u32 crc);

private:
    void setOwnData(u16 inputs, const Vec3 *pos);

    u32 m_data[3];
    u32 m_receivedCRC[12];
};
static_assert(sizeof(RaceFrame) == 0x3c);

struct RaceFramePacket {
    RaceFramePacket(u16 time);
    RaceFramePacket(RaceFrame frame);

    void setPacketReceived(u8 aid, u32 crc);
    bool send();

    u8 magic[2] = {'R', 'F'};
    // We only need 15 bits to represent in-race-time (starting at the countdown).
    // Races are a max of 21600 frames (60 ms * 60 sec * 6 minutes), which can be represented
    // in 15 bits (log2(21600) ~= 14.398). This frees up a bit for the sent flag.
    u16 time : 15;
    bool sent : 1;

    RaceFrame frame;
};
static_assert(sizeof(RaceFramePacket) == 0x40);

class RaceManager {
public:
    void update();

    bool removeRaceFrame(u16 time);

    void pushReceivedPacket(u8 aid, u32 crc);

    static bool isRaceFramePacket(const u8 *message, s32 len);

    static void CreateInstance();

    static void DestroyInstance();

    static RaceManager *Instance() {
        return s_instance;
    }

private:
    RaceFramePacket *getOrCreateFrame(u16 time);
    bool updateTime();
    bool isActive();
    void exportFrame(u32 time);
    void resendFrames();
    s32 findFrame(u16 time);

    SP::CircularBuffer<RaceFramePacket, FrameBufferSize> m_raceFrames;
    u32 m_time;

    static RaceManager *s_instance;
};

} // namespace MKWServer
