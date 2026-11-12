#include "NetManager.hh"

#include "game/host_system/SystemManager.hh"
#include "game/net/GlobeHandler.hh"
#include "game/net/records/Room.hh"
#include "game/system/SaveManager.hh"
#include "game/ui/SectionManager.hh"

#include <revolution/dwc/DWCFriendsMatch.hh>
#include <revolution/dwc/DWCMain.h>
#include <sp/net/ExtendedFriends.hh>
#include <sp/net/mkw_server/MKWServer.hh>

#include <cstring>

extern "C" {
#include <revolution.h>
}

namespace Net {

const MatchMakingInfo &NetManager::currentMMInfo() const {
    return m_matchMakingInfos[m_currMMInfo];
}

u8 NetManager::myAid() const {
    return currentMMInfo().myAid;
}

u32 NetManager::numAids() const {
    return currentMMInfo().numAids;
}

bool NetManager::aidInUse(u8 aid) const {
    return currentMMInfo().availableAids.on(aid);
}

bool NetManager::canSendToAid(u8 aid) const {
    return aidInUse(aid) && aid != myAid();
}

RacePacketHolder *NetManager::lastSentRaceBuffer(u8 aid) {
    return m_sendRacePackets[m_lastSendIdx[aid]][aid];
}

RecordHolder<Header> *NetManager::outgoingBuffer(u8 aid) {
    return m_outgoingRacePacket[aid];
}

SearchRegion NetManager::getSearchRegion() {
    // This would need to be modified for custom regions
    switch (REGION) {
    case REGION_P:
        return SEARCH_REGION_EU;
    case REGION_E:
        return SEARCH_REGION_NA;
    case REGION_J:
        return SEARCH_REGION_JP;
    case REGION_K:
        return SEARCH_REGION_KOR;
    default:
        assert("Invalid region!");
        return SEARCH_REGION_NONE;
    }
}

void NetManager::init(u8 localPlayerCount) {
    REPLACED(init)(localPlayerCount);

    // initialize the unique packets buffer
    m_outgoingUniquePackets.reset();

    // Extended roster (slots 30..63): load from SD for this license. Missing file is fine —
    // rksys.dat's 30 friends stay as they are. See docs/design_docs/ExtendedFriendRoster.md.
    memset(m_extraFriends, 0, sizeof(m_extraFriends));
    if (auto *save = System::SaveManager::Instance()) {
        s16 license = save->currentLicenseIndex();
        if (license >= 0 && license < 4) {
            SP::ExtendedFriends::Load(static_cast<u32>(license));
        } else {
            SP::ExtendedFriends::Clear();
        }
    } else {
        SP::ExtendedFriends::Clear();
    }
}

void NetManager::scheduleShutdown() {
    m_shutdownScheduled = true;

    MKWServer::resetRoomManagerConnection();
}

void NetManager::cancelMatching() {
    // Reset the two race packet handlers active during the globe scene
    if (auto *globeHandler = GlobeHandler::Instance()) {
        globeHandler->reset();
    }
    if (auto *roomHandler = RoomHandler::Instance()) {
        roomHandler->reset();
    }

    // This will exit the InMatchMaking state during the next iteration of the main loop
    m_voteMMSuspension = VoteMatchMakingSuspended::Disconnected;

    // inform wfc-server we're leaving the room
    MKWServer::sendLeaveRoomRequest();
}

bool NetManager::hasFoundMatch() const {
    // We're in a match if my aid is used and theres more than one aid.
    return aidInUse(myAid()) && numAids() > 1;
}

void NetManager::createRacePacket() {
    if (!hasFoundMatch()) {
        return;
    }

    // Don't create a packet if we don't have MKW-Server's address.
    if (!MKWServer::hasMKWServerAddress()) {
        return;
    }
    // Reset the outgoing packets. new frame, new packets
    m_outgoingUniquePackets.reset();

    for (u8 aid = 0; aid < MAX_PLAYER_COUNT; aid++) {
        if (!canSendToAid(aid)) {
            continue;
        }

        // Get the current send buffer then flip it so subsequent record exports work
        u8 sendBufferIdx = m_lastSendIdx[aid];
        m_lastSendIdx[aid] ^= 1;

        // Get the send buffer for this aid
        RacePacketHolder *sendBuffer = m_sendRacePackets[sendBufferIdx][aid];

        // Form a header
        Header header{};
        u32 headerSizes = 0;
        for (u8 i = 0; i < 8; i++) {
            RecordHolder<void> *rh = sendBuffer->holder(i);

            // Header record (index 0) is always 0x10, others use actual size
            u8 size = i == 0 ? sizeof(Header) : rh->recordSize();
            if (size != 0) {
                headerSizes |= 1 << i;
            }
            header.setRecordSize(i, size);
        }

        // Try to lookup the outgoing packet. its unique if -1 is returned
        s32 uniquePacketIdx = m_outgoingUniquePackets.lookup(headerSizes);
        if (uniquePacketIdx != -1) {
            // add this aid as a recipient of this packet, move on to the next aid
            m_outgoingUniquePackets.setRecipient(uniquePacketIdx, aid);
            continue;
        }

        // copy the header to the buffer that actually gets sent
        sendBuffer->header()->copy(&header, sizeof(Header));

        RecordHolder<Header> *outgoing = m_outgoingRacePacket[aid];
        outgoing->reset();

        // copy the records in the sendBuffer to the outgoing buffer
        for (u8 i = 0; i < 8; i++) {
            RecordHolder<void> *record = sendBuffer->holder(i);
            if (record->recordSize() != 0) {
                outgoing->append(record->record(), record->recordSize());
                record->reset();
            }
        }

        // We found a unique packet, so push it to be sent
        if (!m_outgoingUniquePackets.push(outgoing, headerSizes, aid, myAid())) {
            SP_LOG("Pushing a new packet failed!");
        }
    }
}

void NetManager::sendRacePacket() {
    for (u8 i = 0; i < m_outgoingUniquePackets.count(); i++) {
        if (!sendRacePacketToMKWServer(i)) {
            SP_LOG("sendRacePacket(%d) failed!", i);
        }
    }
    m_outgoingUniquePackets.reset();
}

bool NetManager::sendRacePacketToMKWServer(u8 packetIdx) {
    const MKWServer::Packet *outgoingPacket = m_outgoingUniquePackets[packetIdx];

    if (outgoingPacket == nullptr) {
        SP_LOG("Failed to sendRacePacket, outgoingPacket is null!");
        return false;
    }

    if (outgoingPacket->data == nullptr) {
        SP_LOG("Failed to sendRacePacket, outgoingPacket->data is null!");
        return false;
    }

    if (outgoingPacket->data->recordSize() == 0) {
        SP_LOG("Failed to sendRacePacket, outgoingPacket->data->recordSize() is 0!");
        return false;
    }

    // Calc the crc32
    u32 crc32 = NETCalcCRC32(outgoingPacket->data->record(), outgoingPacket->data->recordSize());
    Header *header = reinterpret_cast<Header *>(outgoingPacket->data->record());
    header->crc32 = crc32;

    void *recordToSend = outgoingPacket->data->record();

    // Try to send
    bool sendResult = MKWServer::trySendRacePacketToMKWServer(recordToSend,
            outgoingPacket->data->recordSize());

    // always reset the outgoing buffer
    outgoingPacket->data->reset();
    return sendResult;
}

void NetManager::updateMatchMakingInfoAndRating() {
    if (!MKWServer::recvFromRoomManager()) {
        // return early if we don't have a MatchMakingInfo packet to process
        // base game never returns early; it always tries to set vr/br
        return;
    }

    // Loop over both MatchMakingInfos and set them. In vanilla, calls to this function alternate
    // writing to the two MatchMakingInfos, and XORs m_currMMInfo to do such. It is also the only
    // function that writes to m_currMMInfo. Looping over both MatchMakingInfos when we receive a
    // packet from MKW-Server is sufficient to get match making working in-game.
    for (u8 i = 0; i < 2; i++) {
        auto &mmInfo = m_matchMakingInfos[i];
        MatchMakingInfoPacket *matchMakingInfo = &MKWServer::g_recvMatchPacket;

        mmInfo.availableAids = matchMakingInfo->aidBitmap;
        if (mmInfo.availableAids.none()) {
            mmInfo.reset();
        } else {
            mmInfo.importMKWServerMMInfo(matchMakingInfo);
        }
    }
    auto *userHandler = UserHandler::Instance();
    if (userHandler->prepared()) {
        m_vr = userHandler->myVR();
        m_br = userHandler->myBR();
    }
}

void NetManager::connectToAnybodyAsync() {
    if (MKWServer::connectToRoomManager()) {
        // In vanilla, this function only gets called when searching for public rooms
        // Because of that, we can assume that a non-ww room type is regional and can't be a private
        // room
        bool isWW =
                m_roomType == RoomType::VersusWorldWide || m_roomType == RoomType::BattleWorldWide;
        SearchRegion region = isWW ? SEARCH_REGION_WW : getSearchRegion();

        bool isVS =
                m_roomType == RoomType::VersusWorldWide || m_roomType == RoomType::VersusRegional;
        MKWServer::GameMode mode = isVS ? MKWServer::GameMode::VS : MKWServer::GameMode::Battle;

        if (!MKWServer::sendSearchRoomRequest(region, mode)) {
            SP_LOG("Search room request failed! Region: %d, Mode: %d", region,
                    static_cast<u32>(mode));
        }
    }
}

void NetManager::resetFriendData(u32 friendIdx) {
    if (friendIdx >= MAX_FRIEND_COUNT) {
        return;
    }
    FriendInfo &info = friendAt(friendIdx);
    info.statusData.roomId = 0;
    info.statusData.regionId = 0;
    info.statusData.status = 0;
    info.statusData.playerCount = 0;
    info.statusData.currRace = 0;
    info.dwcFriendStatus = 0;
    info.addedBack = true;
}

void NetManager::resetFriends() {
    for (u32 i = 0; i < MAX_FRIEND_COUNT; i++) {
        resetFriendData(i);
    }
}

void NetManager::setFriendsAddedBack(u32 *pidsThatHaveAddedBack, u32 countOfPids) {
    // Vanilla only walks 0..29.
    REPLACED(setFriendsAddedBack)(pidsThatHaveAddedBack, countOfPids);

    for (u32 i = VANILLA_FRIEND_COUNT; i < MAX_FRIEND_COUNT; i++) {
        FriendInfo &info = friendAt(i);
        info.addedBack = false;

        if (!SP::ExtendedFriends::s_slots[i - VANILLA_FRIEND_COUNT].occupied()) {
            info.addedBack = true;
            continue;
        }

        s32 pid = DWCi_GetProfileIDFromList(static_cast<s32>(i));
        if (pid <= 0 || !pidsThatHaveAddedBack) {
            continue;
        }
        for (u32 j = 0; j < countOfPids; j++) {
            if (pidsThatHaveAddedBack[j] == static_cast<u32>(pid)) {
                info.addedBack = true;
                break;
            }
        }
    }
}

void NetManager::updateStatusDatas() {
    // Vanilla loops DWC friendListLen into m_friends[30]. Clamp to 30 for that pass, then
    // fill extended slots ourselves so indices 30..63 stay in sync with s_dwcList.
    int savedLen = -1;
    if (s_dwcControl) {
        savedLen = s_dwcControl->friendControl.friendListLen;
        s_dwcControl->friendControl.friendListLen = VANILLA_FRIEND_COUNT;
    }
    REPLACED(updateStatusDatas)();
    if (s_dwcControl && savedLen >= 0) {
        s_dwcControl->friendControl.friendListLen = savedLen;
    }

    for (u32 i = VANILLA_FRIEND_COUNT; i < MAX_FRIEND_COUNT; i++) {
        FriendInfo &info = friendAt(i);
        if (!SP::ExtendedFriends::s_slots[i - VANILLA_FRIEND_COUNT].occupied()) {
            info.dwcFriendStatus = 0;
            memset(&info.statusData, 0, sizeof(info.statusData));
            continue;
        }

        int size = static_cast<int>(sizeof(StatusData));
        StatusData data{};
        u8 dwcStatus = DWC_GetFriendStatusData(&SP::ExtendedFriends::s_dwcList[i],
                reinterpret_cast<char *>(&data), &size);
        info.dwcFriendStatus = dwcStatus;
        if (size > 0) {
            u32 copy = static_cast<u32>(size);
            if (copy > sizeof(StatusData)) {
                copy = sizeof(StatusData);
            }
            memcpy(&info.statusData, &data, copy);
        }
    }
}

FriendJoinableStatus NetManager::getFriendJoinableStatus(u32 friendId) const {
    if (friendId >= MAX_FRIEND_COUNT) {
        return FriendJoinableStatus::Offline;
    }

    const FriendInfo &info = friendAt(friendId);
    if (!info.addedBack || info.dwcFriendStatus == 0) {
        return FriendJoinableStatus::Offline;
    }

    auto friendStatus = static_cast<FriendStatus>(info.statusData.status);

    if (friendStatus != FriendStatus::Idle) {
        switch (friendStatus) {
        case FriendStatus::PlayingPublicVS:
            if (m_disconnectPenalty != 0) {
                return FriendJoinableStatus::Online;
            }
            if (info.statusData.regionId != -1) {
                if (info.statusData.regionId ==
                        static_cast<s8>(System::SystemManager::Instance()->matchingArea())) {
                    return FriendJoinableStatus::JoinableRegionalVS;
                }
                return FriendJoinableStatus::UnjoinableRegionalVS;
            }
            return FriendJoinableStatus::WorldWideVersus;

        case FriendStatus::PlayingPublicBattle:
            if (m_disconnectPenalty != 0) {
                return FriendJoinableStatus::Online;
            }
            if (info.statusData.regionId != -1) {
                if (info.statusData.regionId ==
                        static_cast<s8>(System::SystemManager::Instance()->matchingArea())) {
                    return FriendJoinableStatus::JoinableRegionalBattle;
                }
                return FriendJoinableStatus::UnjoinableRegionalBattle;
            }
            return FriendJoinableStatus::WorldWideBattle;

        case FriendStatus::HostingVS:
        case FriendStatus::HostingBattle:
        case FriendStatus::GuestInVSFriendRoom:
        case FriendStatus::GuestInBattleFriendRoom:
        case FriendStatus::Online:
        case FriendStatus::OpenRoom:
        case FriendStatus::PlayingWithFriends:
            return static_cast<FriendJoinableStatus>(friendStatus);

        default:
            return static_cast<FriendJoinableStatus>(friendStatus);
        }
    }
    return FriendJoinableStatus::Online;
}

void NetManager::connectToGameServerFromGroupId() {
    u32 friendId = currentMMInfo().hostFriendId;
    FriendJoinableStatus status = getFriendJoinableStatus(friendId);
    SearchRegion searchRegion = SEARCH_REGION_NONE;

    switch (status) {
    case FriendJoinableStatus::WorldWideVersus:
    case FriendJoinableStatus::WorldWideBattle:
        searchRegion = SEARCH_REGION_WW;
        break;
    case FriendJoinableStatus::JoinableRegionalVS:
    case FriendJoinableStatus::JoinableRegionalBattle:
        searchRegion = getSearchRegion();
        break;
    default:
        SP_LOG("Mismatching search region as friend trying to join!");
        break;
    }

    s32 friendProfileId = DWCi_GetProfileIDFromList(friendId);
    if (MKWServer::connectToRoomManager()) {
        MKWServer::sendJoinFriendRequest(friendProfileId, searchRegion);
    }
}

void NetManager::processRacePacket(u8 aid, u8 *packet, u32 size) {
    Header *header = reinterpret_cast<Header *>(packet);
    u32 origCrc32 = header->crc32;
    header->crc32 = 0;
    u32 calcCrc32 = NETCalcCRC32(header, size);

    // make sure the packet isn't corrupted
    if (origCrc32 == calcCrc32) {
        // data for other packet is right after the header, so add the
        // packet[i] size to this to get a specific offset
        u8 *dataPacketPtr = reinterpret_cast<u8 *>(header);
        for (u32 i = 0; i < std::size(header->recordSizes); i++) {
            if (header->recordSizes[i] != 0) {
                // reset and copy the recieved packet into recv structs
                m_recvRacePackets[m_lastRecvIdx[aid][i] ^ 1][aid]->holder(i)->reset();
                m_recvRacePackets[m_lastRecvIdx[aid][i] ^ 1][aid]->holder(i)->copy(dataPacketPtr,
                        header->recordSizes[i]);

                // increment the data pointer to the next packet offset
                dataPacketPtr += header->recordSizes[i];

                // flip the last recieved buffer idx
                m_lastRecvIdx[aid][i] ^= 1;
            }
        }
    } else {
        SP_LOG("Invalid Checksum!");
    }
}

void NetManager::updateAddedFriendsCallback(void *r3, void *r4, void *r5) {
    REPLACED(updateAddedFriendsCallback)(r3, r4, r5);

    // Persist extended slots whenever the roster changes. No-op until extras are occupied.
    if (auto *save = System::SaveManager::Instance()) {
        s16 license = save->currentLicenseIndex();
        if (license >= 0 && license < 4 && SP::ExtendedFriends::OccupiedCount() > 0) {
            SP::ExtendedFriends::Save(static_cast<u32>(license));
        }
    }

    if (!MKWServer::connectToRoomManager()) {
        SP_LOG("Failed to connect to room manager!");
        return;
    }

    u8 localPlayerCount = UI::SectionManager::Instance()->getLocalPlayerCount();
    bool sendResult = MKWServer::sendLocalPlayerCount(localPlayerCount);
    if (!sendResult) {
        SP_LOG("Sending localPlayerCount failed!");
    }
}

} // namespace Net
