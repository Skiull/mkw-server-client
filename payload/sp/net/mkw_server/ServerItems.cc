#include "ServerItems.hh"

extern "C" {
#include <revolution.h>
}

#include <sp/net/mkw_server/MKWServer.hh>

#include <string.h>

namespace MKWServer::ServerItems {

// Resend a request that hasn't been answered after this long.
static constexpr u32 RESEND_INTERVAL_MS = 250;
// Give up on a request (no item) after this long.
static constexpr u32 REQUEST_TIMEOUT_MS = 2000;

static constexpr char ITEM_REQUEST_MAGIC[] = "IR";
static constexpr char ITEM_RESPONSE_MAGIC[] = "IT";

struct Request {
    bool active;
    bool answered;
    u8 seq;
    u8 item;
    OSTime startedAt;
    OSTime lastSentAt;
};

static Request s_requests[MAX_LOCAL_PLAYERS] = {};
// Last seq used per local player. The first request uses 1.
static u8 s_lastSeq[MAX_LOCAL_PLAYERS] = {};

static OSTime millisecondsToTicks(u32 ms) {
    return OSMillisecondsToTicks(static_cast<OSTime>(ms));
}

static void send(u8 playerIdx, Request &request) {
    u8 packet[ITEM_REQUEST_LEN];
    packet[0] = ITEM_REQUEST_MAGIC[0];
    packet[1] = ITEM_REQUEST_MAGIC[1];
    packet[2] = playerIdx;
    packet[3] = request.seq;
    if (!trySendRacePacketToMKWServer(packet, sizeof(packet))) {
        SP_LOG("Failed to send item request (player %u, seq %u)", playerIdx, request.seq);
    }
    request.lastSentAt = OSGetTime();
}

void request(u8 playerIdx) {
    if (playerIdx >= MAX_LOCAL_PLAYERS) {
        return;
    }

    Request &request = s_requests[playerIdx];
    request.active = true;
    request.answered = false;
    request.seq = ++s_lastSeq[playerIdx];
    request.item = NO_ITEM;
    request.startedAt = OSGetTime();
    send(playerIdx, request);
}

Status poll(u8 playerIdx, u8 *item) {
    if (playerIdx >= MAX_LOCAL_PLAYERS) {
        return Status::None;
    }

    Request &request = s_requests[playerIdx];
    if (!request.active) {
        return Status::None;
    }

    if (request.answered) {
        request.active = false;
        if (request.item == NO_ITEM) {
            return Status::Refused;
        }
        *item = request.item;
        return Status::Granted;
    }

    OSTime now = OSGetTime();
    if (now - request.startedAt >= millisecondsToTicks(REQUEST_TIMEOUT_MS)) {
        SP_LOG("Item request timed out (player %u, seq %u)", playerIdx, request.seq);
        request.active = false;
        return Status::TimedOut;
    }

    if (now - request.lastSentAt >= millisecondsToTicks(RESEND_INTERVAL_MS)) {
        send(playerIdx, request);
    }

    return Status::Waiting;
}

void cancel(u8 playerIdx) {
    if (playerIdx < MAX_LOCAL_PLAYERS) {
        s_requests[playerIdx].active = false;
    }
}

static bool isValidItem(u8 item) {
    // ItemId::Green (0x00) to ItemId::TripNanas (0x12), or no item. 0x13 is unused.
    return item <= 0x12 || item == NO_ITEM;
}

static void handleItemResponse(u8 playerIdx, u8 seq, u8 item) {
    if (playerIdx >= MAX_LOCAL_PLAYERS) {
        SP_LOG("Item response for invalid player %u", playerIdx);
        return;
    }

    Request &request = s_requests[playerIdx];
    // Late answers (to a request we gave up on or replaced) and duplicates are ignored.
    if (!request.active || request.answered || request.seq != seq) {
        return;
    }

    if (!isValidItem(item)) {
        SP_LOG("Item response with invalid item %u, treating as refused", item);
        item = NO_ITEM;
    }

    request.answered = true;
    request.item = item;
}

} // namespace MKWServer::ServerItems

bool ServerItems_isItemResponsePacket(const u8 *message, s32 len) {
    return len == static_cast<s32>(MKWServer::ServerItems::ITEM_RESPONSE_LEN) &&
            memcmp(message, MKWServer::ServerItems::ITEM_RESPONSE_MAGIC, 2) == 0;
}

void ServerItems_handleItemResponsePacket(const u8 *message, s32 len) {
    if (!ServerItems_isItemResponsePacket(message, len)) {
        return;
    }
    MKWServer::ServerItems::handleItemResponse(message[2], message[3], message[4]);
}
