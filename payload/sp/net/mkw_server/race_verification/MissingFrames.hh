#pragma once

#include <Common.hh>

namespace MKWServer {

static constexpr u16 MissingFramesMagic = 0x4D46; // "MF"
static constexpr size_t MissingFramesHeaderSize = 4;

// This request has a dynamic size depending on the number of frames that are missing.
// So no member in this struct.
// Request format:
// 0x0: u16 magic "MF"
// 0x2: u16 count
// 0x4: u16 frames[count]
struct MissingFramesRequest {
    static bool IsMissingFramesRequest(const u8 *message, s32 len) {
        if (!message || len < static_cast<s32>(MissingFramesHeaderSize)) {
            return false;
        }

        return ReadU16(message) == MissingFramesMagic;
    }

    static u16 Count(const u8 *message) {
        return ReadU16(message + 2);
    }

    static u16 TimeAt(const u8 *message, u32 index) {
        return ReadU16(message + MissingFramesHeaderSize + index * sizeof(u16));
    }

    static bool IsWellFormed(const u8 *message, s32 len) {
        u32 count = Count(message);
        return len == static_cast<s32>(MissingFramesHeaderSize + count * sizeof(u16));
    }

private:
    static u16 ReadU16(const u8 *p) {
        return (static_cast<u16>(p[0]) << 8) | p[1];
    }
};

struct MissingFramesAck {
    static constexpr u16 Magic = 0x4D41; // "MA"
    static bool IsAck(const u8 *message, s32 len) {
        return len == 2 && ((static_cast<u16>(message[0]) << 8) | message[1]) == Magic;
    }
};

} // namespace MKWServer
