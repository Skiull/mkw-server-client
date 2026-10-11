#pragma once

#include <Common.hh>

#include <common/TVec3.hh>

namespace MKWServer {

static constexpr u16 RaceFrameMagic = 0x5246; // "RF"
static constexpr size_t RaceFrameAckSize = 4;

// RaceFramePlayer contains info of a local players' frame needed to replay it.
struct __attribute__((packed)) RaceFramePlayer {
    RaceFramePlayer() = default;
    RaceFramePlayer(u16 inputs, const Vec3 &pos) : inputs(inputs), pos{pos.x, pos.y, pos.z} {}

    // Players' inputs. See RaceInputState::inputBitfield().
    u16 inputs;

    // Players' position used to verify physics.
    f32 pos[3];
};
static_assert(sizeof(RaceFramePlayer) == 0xe);

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
static_assert(sizeof(RaceFrame) == 0x50);

} // namespace MKWServer
