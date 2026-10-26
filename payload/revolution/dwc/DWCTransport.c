#include "DWCTransport.h"

#include <game/net/RaceHandler.h>
#include <game/system/RaceManager.h>

#include <sp/net/mkw_server/MKWServer.h>
#include <sp/net/mkw_server/ServerItems.h>

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
        return GT2False;
    }

    if (verifySearchIdMagic(message, len)) {
        handleSearchIdPacket(message, len);
        return GT2True;
    }

    if (isReadyAckPacket(message, len)) {
        SP_LOG("Received ready ACK packet from mkw-server!");
        RaceHandler_setAckReady();
        return GT2True;
    }

    if (isStartPacket(message, len)) {
        SP_LOG("Received Start packet from mkw-server!");
        RaceManager_startCountdown();
        return GT2True;
    }

    if (isPingPacket(message, len)) {
        sendPong();
        return GT2True;
    }

    if (isPingTimePacket(message, len)) {
        setPingTime(message);
        return GT2True;
    }

    if (ServerItems_isItemResponsePacket(message, len)) {
        ServerItems_handleItemResponsePacket(message, len);
        return GT2True;
    }

    MKWServerPacketType messageType = message[0];
    switch (messageType) {
    case MKW_SERVER_RACE_PACKET:
        if (s_dwcTransport == NULL || s_dwcTransport->userRecvCallback == NULL) {
            return FALSE;
        }

        // the aid is in offset 0x1
        u8 aid = message[1];

        s_dwcTransport->userRecvCallback(aid, message, len);
        return TRUE;

    default:
        return REPLACED(DWCi_GT2UnrecognizedMessageCallback)(socket, ip, port, message, len);
    }
    return FALSE;
}
