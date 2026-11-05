#include "Mogi.hh"

#include <sp/net/mkw_server/MKWServer.hh>

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

// In mogi mode the friends page's "Create a Room" and "Find a Friend" read "Create a Mogi" and
// "Find a Mogi" (TextBox_setMessage hook, sp/net/mkw_server/Mogi.S).
extern "C" u32 MogiRemapMessage(u32 messageId) {
    if (!s_createMogi) {
        return messageId;
    }
    switch (messageId) {
    case 4101:
        return 10025;
    case 5001:
        return 10026;
    default:
        return messageId;
    }
}

} // namespace MKWServer
