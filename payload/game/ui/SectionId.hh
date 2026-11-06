// See LICENSE-mkw-sp

#pragma once

namespace UI {

enum class SectionId {
    None = -1,

    TA = 0x1F,
    VS1P = 0x20,
    VS2P = 0x21,
    VS3P = 0x22,
    VS4P = 0x23,
    TeamVS1P = 0x24,
    TeamVS2P = 0x25,
    TeamVS3P = 0x26,
    TeamVS4P = 0x27,
    Battle1P = 0x28,
    Battle2P = 0x29,
    Battle3P = 0x2A,
    Battle4P = 0x2B,
    GhostTA = 0x30,
    GhostTAOnline = 0x31,

    WifiSingle = 0x55,
    WifiSingleDC = 0x56,
    WifiSingleFriend = 0x57,

    WifiMulti = 0x5b,
    WifiMultiDC = 0x5c,
    WifiMultiFriend = 0x5d,

    Max = 0x95,
};

} // namespace UI
