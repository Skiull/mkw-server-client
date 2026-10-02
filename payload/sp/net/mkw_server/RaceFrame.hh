#pragma once

#include <Common.hh>

#include <common/TVec3.hh>

namespace MKWServer {

static constexpr u16 RaceFrameMagic = 0x5246; // "RF"
static constexpr size_t RaceFrameAckSize = 4;

// RaceFrame contains information about a given frame needed to replay it.
// TODO: Pack members as much as possible.
// TODO: Handle local multiplayer.
struct RaceFrame {
    RaceFrame(u16 time, u32 inputs, const Vec3 &pos) : time(time), inputs(inputs), pos(pos) {}

    static bool IsRaceFrameAck(const u8 *message, s32 len) {
        if (!message || len != static_cast<s32>(RaceFrameAckSize)) {
            return false;
        }

        u16 magic = (static_cast<u16>(message[0]) << 8) | message[1];
        return magic == RaceFrameMagic;
    }

    u16 magic = RaceFrameMagic;
    // Time in frames since the countdown started.
    u16 time;
    // Our inputs. See RaceInputState::inputBitfield().
    u32 inputs = 0;
    // Our position. Used to verify physics.
    Vec3 pos = {};
    // The time each aid sent a race packet to us.
    u32 recvPacketTimestamps[12] = {};
};
static_assert(sizeof(RaceFrame) == 0x44);

} // namespace MKWServer
