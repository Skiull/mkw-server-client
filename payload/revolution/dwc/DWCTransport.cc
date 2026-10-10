#include "DWCTransport.hh"

#include <game/net/RaceHandler.hh>
#include <game/system/RaceManager.hh>

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/net/mkw_server/race_verification/FinishTime.hh>
#include <sp/net/mkw_server/race_verification/MissingFrames.hh>

#include <string.h>

#define START_MESSAGE "ST"
#define PING_MESSAGE "PI"
#define READY_ACK_MESSAGE "RA"
#define PING_TIME "PT"
// TODO: Don't hardcode size.
#define PING_TIME_PACKET_LEN 7

bool isReadyAckPacket(const u8 *message, s32 len) {
    return len == strlen(READY_ACK_MESSAGE) && memcmp(message, READY_ACK_MESSAGE, len) == 0;
}

bool isStartPacket(const u8 *message, s32 len) {
    return len == strlen(START_MESSAGE) && memcmp(message, START_MESSAGE, len) == 0;
}

bool isPingPacket(const u8 *message, s32 len) {
    return len == strlen(PING_MESSAGE) && memcmp(message, PING_MESSAGE, len) == 0;
}

bool isPingTimePacket(const u8 *message, s32 len) {
    // This packet contains data. Check the packet length, not the length of the magic.
    return len == PING_TIME_PACKET_LEN && memcmp(message, PING_TIME, strlen(PING_TIME)) == 0;
}

BOOL DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, u32 ip, u16 port, const u8 *message,
        s32 len) {
    if (message == NULL || len == 0) {
        SP_LOG("GT2 Unrecognized : Null message or zero length.");
        return FALSE;
    }

    if (MKWServer::verifySearchIdMagic(message, len)) {
        MKWServer::handleSearchIdPacket(message, len);
        return TRUE;
    }

    if (isReadyAckPacket(message, len)) {
        SP_LOG("Received ready ACK packet from mkw-server!");
        if (auto *raceHandler = Net::RaceHandler::Instance()) {
            raceHandler->setAckReady();
        } else {
            SP_LOG("Can't call RaceHandler::setAckReady(). RaceHandler is nullptr!");
        }
        return TRUE;
    }

    if (isStartPacket(message, len)) {
        SP_LOG("Received Start packet from mkw-server!");
        if (auto *raceManager = System::RaceManager::Instance()) {
            raceManager->startCountdown();
        } else {
            SP_LOG("Can't call RaceManager::setAckReady(). RaceManager is nullptr!");
        }
        return TRUE;
    }

    if (isPingPacket(message, len)) {
        MKWServer::sendPong();
        return TRUE;
    }

    if (isPingTimePacket(message, len)) {
        MKWServer::setPingTime(message);
        return TRUE;
    }

    if (MKWServer::FinishTime::IsFinishTimeAck(message, len)) {
        if (auto *raceManager = MKWServer::RaceManager::Instance()) {
            raceManager->onFinishTimeAck();
        }
        return TRUE;
    }

    if (MKWServer::RaceFrame::IsRaceFrameAck(message, len)) {
        auto *raceManager = MKWServer::RaceManager::Instance();
        if (!raceManager) {
            SP_LOG("RaceFrameAck sent but MKWServer::RaceManager is nullptr!");
        }

        u16 time = (static_cast<u16>(message[2]) << 8) | message[3];
        return raceManager->removeRaceFrame(time);
    }

    if (MKWServer::MissingFramesRequest::IsMissingFramesRequest(message, len)) {
        if (auto *raceManager = MKWServer::RaceManager::Instance()) {
            raceManager->handleMissingFrames(message, len);
        } else {
            SP_LOG("Missing frames request sent but MKWServer::RaceManager is nullptr!");
        }
        return TRUE;
    }

    if (MKWServer::MissingFramesAck::IsAck(message, len)) {
        if (auto *raceManager = MKWServer::RaceManager::Instance()) {
            raceManager->onMissingFramesAck();
        }
        return TRUE;
    }

    MKWServer::PacketType messageType = static_cast<MKWServer::PacketType>(message[0]);
    switch (messageType) {
    case MKWServer::PacketType::RacePacket: {
        if (s_dwcTransport == NULL || s_dwcTransport->userRecvCallback == NULL) {
            return FALSE;
        }

        // the aid is in offset 0x1
        u8 aid = message[1];

        s_dwcTransport->userRecvCallback(aid, message, len);
        return TRUE;
    }
    default:
        return REPLACED(DWCi_GT2UnrecognizedMessageCallback)(socket, ip, port, message, len);
    }
    return FALSE;
}
