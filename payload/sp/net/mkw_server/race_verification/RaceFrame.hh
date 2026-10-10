#pragma once

#include <Common.hh>

#include <common/TVec3.hh>

namespace MKWServer {

static constexpr u16 RaceFrameMagic = 0x5246; // "RF"
static constexpr size_t RaceFrameAckSize = 4;

// RaceFramePlayer contains info of a local players' frame needed to replay it.
struct RaceFramePlayer {
    RaceFramePlayer() = default;
    RaceFramePlayer(u32 inputs, const Vec3 &pos) : inputs(inputs), pos(pos) {}

    // Players' inputs. See RaceInputState::inputBitfield().
    u32 inputs;

    // Players' position used to verify physics.
    Vec3 pos;
};
static_assert(sizeof(RaceFramePlayer) == 0x10);

// RaceFrame holds received packets and player info on a single frame.
// TODO: Pack members as much as possible.
struct RaceFrame {
    RaceFrame() = default;
    RaceFrame(u16 time) : time(time) {}

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

    // The time each aid sent a race packet to us.
    u32 recvPacketTimestamps[12];

    // Data for each local player.
    RaceFramePlayer players[2];
};
static_assert(sizeof(RaceFrame) == 0x54);

} // namespace MKWServer
