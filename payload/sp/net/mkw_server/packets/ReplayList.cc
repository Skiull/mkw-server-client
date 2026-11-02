#include "ReplayList.hh"

#include <string.h>

namespace MKWServer {

u8 s_replayIndex = 0;

static ReplayListPacket s_list{};
static u32 s_serial = 0;

void processReplayListPacket(const u8 *data) {
    memcpy(&s_list, data, sizeof(s_list));
    s_serial++;
}

const ReplayListPacket &latestReplayList() {
    return s_list;
}

u32 replayListSerial() {
    return s_serial;
}

} // namespace MKWServer
