#include "DWCMain.hh"

#include <sp/net/mkw_server/RoomManager.hh>
#include <sp/net/mkw_server/packets/MatchMakingInfo.hh>

u32 DWC_GetNumConnectionsHost() {
    return MKWServer::mmInfo()->numAids;
}

u32 DWC_GetGroupId() {
    return MKWServer::mmInfo()->roomId;
}

u8 DWC_GetSuspendMatch() {
    return MKWServer::mmInfo()->suspended;
}

u8 DWC_IsValidMatchCancel() {
    return MKWServer::mmInfo()->canceled;
}

bool DWC_RequestSuspendMatchAsync(bool suspendVote) {
    if (MKWServer::connectToRoomManager()) {
        return MKWServer::sendSuspendRequest(suspendVote);
    }
    return false;
}
