#include "OpenHost.hh"

extern "C" {
#include <revolution/dwc/DWCMatch.h>
#include <revolution/gamespy/gp/gp.h>
}

#include <sp/net/mkw_server/RoomManager.hh>
#include <sp/settings/Settings.hh>

namespace SP::OpenHost {

namespace {

// What this GameSpy connection was last told: -1 = nothing yet.
s32 s_sent = -1;
GPConnection *s_sentConnection = nullptr;

} // namespace

void Calc() {
    // Only once logged in: before that, s_dwcMatch can be stale from an earlier session and
    // writing through it corrupted memory (crash when connecting). The room manager connects
    // after the WFC login.
    GPConnection *connection = nullptr;
    if (MKWServer::isRoomManagerConnected() && s_dwcMatch) {
        // DWC's match control starts with the GameSpy connection (the WFC payload reads it
        // the same way for its own Open Host prompt).
        connection = *reinterpret_cast<GPConnection **>(s_dwcMatch);
    }
    if (!connection || !*connection) {
        s_sent = -1;
        s_sentConnection = nullptr;
        return;
    }
    if (connection != s_sentConnection) {
        s_sent = -1;
        s_sentConnection = connection;
    }

    s32 want = Settings::getSetting<Settings::Setting::OpenHost>() == Settings::OpenHost::Enabled;
    if (want == s_sent) {
        return;
    }
    // GameSpy sends the updatepro buffer on its next process.
    if (gpiAppendStringToBuffer(connection, &(*connection)->updateproBuffer, "\\wl:oh\\") != 0 ||
            gpiAppendStringToBuffer(connection, &(*connection)->updateproBuffer,
                    want ? "1" : "0") != 0) {
        return; // Try again next frame.
    }
    SP_LOG("OpenHost: sent wl:oh %d", want);
    s_sent = want;
}

} // namespace SP::OpenHost
