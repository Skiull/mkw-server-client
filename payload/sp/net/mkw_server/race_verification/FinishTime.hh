#pragma once

#include <Common.hh>

namespace MKWServer {

static constexpr u16 FinishTimeMagic = 0x4654; // "FT"

struct FinishTime {
    FinishTime(u16 time) : time(time) {}

    static bool IsFinishTimeAck(const u8 *message, s32 len) {
        if (!message || len != sizeof(FinishTimeMagic)) {
            return false;
        }

        u16 magic = (static_cast<u16>(message[0]) << 8) | message[1];
        return magic == FinishTimeMagic;
    }

    u16 magic = FinishTimeMagic;
    u16 time;
};
static_assert(sizeof(FinishTime) == 0x4);

} // namespace MKWServer
