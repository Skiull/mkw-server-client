#include "DWCAuth.hh"

#include <sp/net/WiiLink.hh>

extern "C" {
#include <revolution.h>
#include <revolutionex/nhttp.h>
}

void DWCi_Auth_SendRequest(u32 authStage, wchar_t *miiName, char *gameId, u32 r6, u64 userId) {
    // call the original function if the payload is ready
    if (MKWServer::wwfcPayloadReady()) {
        REPLACED(DWCi_Auth_SendRequest)(authStage, miiName, gameId, r6, userId);
        return;
    }

    NHTTPRequestHandle request = MKWServer::createWFCAuthRequest();
    if (request == NULL) {
        return;
    }
    s_DWCAuthControl->handle = NHTTPSendRequestAsync(request);
}
