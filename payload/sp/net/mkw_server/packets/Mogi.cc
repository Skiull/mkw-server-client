#include "Mogi.hh"

namespace MKWServer {

static bool s_linked = false;
static u32 s_serial = 0;

void processMogiListPacket(const u8 *data) {
    s_linked = data[0] != 0;
    s_serial++;
}

bool mogiLinked() {
    return s_linked;
}

u32 mogiListSerial() {
    return s_serial;
}

} // namespace MKWServer
