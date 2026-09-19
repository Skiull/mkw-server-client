#pragma once

#include <Common.hh>
#include <common/TVec3.hh>

#include "sp/CircularBuffer.hh"

namespace MKWServer {

static constexpr u8 RaceFrameMagic[2] = {'R', 'F'};

static constexpr size_t FrameBufferSize = 1200;

class __attribute__((packed)) RaceFrame {
public:
    RaceFrame(u16 time);

    u16 time() const;

private:
    void set(u16 inputs, const Vec3 *pos);

    u16 m_time;
    u32 m_data[3];
};
static_assert(sizeof(RaceFrame) == 0xe);

struct RaceFramePacket {
    RaceFramePacket(u16 time);
    RaceFramePacket(RaceFrame frame);

    u8 magic[2] = {'R', 'F'};
    RaceFrame frame;
};
static_assert(sizeof(RaceFramePacket) == 0x10);

class RaceManager {
public:
    void update(u16 time);

    bool removeRaceFrame(u16 time);

    static bool isRaceFramePacket(const u8 *message, s32 len);

    static void CreateInstance();

    static void DestroyInstance();

    static RaceManager *Instance() {
        return s_instance;
    }

private:
    bool sendFrame(const RaceFramePacket *frame) const;

    // Creates and sends RaceFramePackets to mkw-server.
    void updateCurrentFrame(u16 time);

    // Resends old frames that haven't been ack-ed yet by mkw-server.
    void resendOldFrames(u16 time);

    s32 findFrame(u16 time);

    SP::CircularBuffer<RaceFrame, FrameBufferSize> m_raceFrames;

    static RaceManager *s_instance;
};

} // namespace MKWServer
