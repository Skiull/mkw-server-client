#pragma once

#include <Common.hh>

namespace Net {

// Tentative name. Builds the outgoing ITEM records and runs the item handshake with the other
// players in the room: when a local player's roulette starts, the rolled item is written to their
// ITEM record in request mode, and the roulette waits until the room has answered it.
class ItemHandler {
public:
    // 0x8065c6c0
    // Writes a roulette request for item into playerId's ITEM record (held item = item,
    // mode = 1) and starts the handshake. Only called by ItemRoulette::start (0x807b9fb0).
    // Hooked so the request can be held back while the server decides the item.
    REPLACE void requestItem(u32 playerId, u32 item);
    void REPLACED(requestItem)(u32 playerId, u32 item);

    // Added. Until endHoldRequests(), requestItem() doesn't write anything and only records
    // which ItemHandler it was called on.
    static void beginHoldRequests();

    // Added. Ends beginHoldRequests() and returns the ItemHandler that requestItem() was called
    // on in the meantime, or nullptr if it wasn't called (the game isn't online).
    static ItemHandler *endHoldRequests();
};

} // namespace Net
