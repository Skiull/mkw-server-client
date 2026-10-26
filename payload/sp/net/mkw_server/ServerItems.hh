#pragma once

// Server-sided item decisions. See docs/server-item-decisions.md in mkw-server.
//
// Client -> server, on touching an item box: 'I' 'R' playerIdx seq       (4 bytes)
// Server -> client, the decided item:        'I' 'T' playerIdx seq item  (5 bytes)
//
// playerIdx is 0 for the main player and 1 for a guest. A lost response is recovered by
// resending the same request (same seq); the server answers it again instead of rolling again.
// Item 0x14 (no item) means the request was refused.

#include <Common.hh>

extern "C" {
#include "ServerItems.h"
}

namespace MKWServer::ServerItems {

constexpr u32 MAX_LOCAL_PLAYERS = 2;
constexpr u32 ITEM_REQUEST_LEN = 4;
constexpr u32 ITEM_RESPONSE_LEN = 5;
constexpr u8 NO_ITEM = 0x14;

enum class Status {
    None,     // No request in progress.
    Waiting,  // Request sent, no answer yet.
    Granted,  // The server answered with an item.
    Refused,  // The server answered with no item.
    TimedOut, // No answer in time.
};

// Starts a new request for a local player and sends it. Replaces any request in progress.
void request(u8 playerIdx);

// Resends the request when due and reports its status. Granted, Refused and TimedOut are
// reported once, after which the request is over. item is set for Granted.
Status poll(u8 playerIdx, u8 *item);

// Drops the request in progress, if any. Late answers to it are ignored.
void cancel(u8 playerIdx);

} // namespace MKWServer::ServerItems
