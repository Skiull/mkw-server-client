#include "ItemRoulette.hh"

#include "game/kart/KartItem.hh"
#include "game/net/ItemHandler.hh"
#include "game/net/RaceHandler.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/net/mkw_server/ServerItems.hh>

namespace Item {

namespace ServerItems = MKWServer::ServerItems;

// A roulette whose item is decided by mkw-server, one per local player.
struct ServerRoulette {
    ItemRoulette *roulette;
    // The server has answered, item is what the roulette lands on.
    bool granted;
    ItemId item;
    // Online: the ItemHandler the game would have sent the item request with.
    Net::ItemHandler *itemHandler;
    // m_noEarlyStop before we forced it on.
    u8 noEarlyStop;
};

static ServerRoulette s_serverRoulettes[ServerItems::MAX_LOCAL_PLAYERS] = {};

static s32 findServerRoulette(const ItemRoulette *roulette) {
    for (u32 i = 0; i < ServerItems::MAX_LOCAL_PLAYERS; i++) {
        if (s_serverRoulettes[i].roulette == roulette) {
            return i;
        }
    }
    return -1;
}

static void releaseServerRoulette(s32 playerIdx) {
    ServerItems::cancel(playerIdx);
    s_serverRoulettes[playerIdx] = {};
}

// Returns the local player index whose items mkw-server decides, or -1.
static s32 getServerPlayerIdx(u8 playerId) {
    if (!MKWServer::hasMKWServerAddress()) {
        return -1;
    }
    auto *raceHandler = Net::RaceHandler::Instance();
    if (!raceHandler) {
        return -1;
    }
    s32 playerIdx = raceHandler->getLocalPlayerIdx(playerId);
    if (playerIdx < 0 || playerIdx >= static_cast<s32>(ServerItems::MAX_LOCAL_PLAYERS)) {
        return -1;
    }
    return playerIdx;
}

void ItemRoulette::start(u32 r4, u32 r5, u32 r6, u32 r7) {
    s32 playerIdx = getServerPlayerIdx(m_owner->playerId());
    if (playerIdx < 0) {
        REPLACED(start)(r4, r5, r6, r7);
        return;
    }

    // The game rolls m_item locally and, online, writes it to the ITEM record to start the item
    // handshake. Hold that request back: the record must only ever show items the server
    // granted. The handshake is started with the server's item once it answers.
    bool wasIdle = m_state == State::Idle;
    Net::ItemHandler::beginHoldRequests();
    REPLACED(start)(r4, r5, r6, r7);
    Net::ItemHandler *itemHandler = Net::ItemHandler::endHoldRequests();
    if (!wasIdle || m_state == State::Idle) {
        // The roulette didn't start (already spinning, or the game refused).
        return;
    }

    // A previous roulette for this player that never finished (shouldn't happen).
    if (s32 previous = findServerRoulette(this); previous >= 0) {
        releaseServerRoulette(previous);
    }

    // Nothing was requested, so there's nothing to wait for yet.
    m_waitingForHandshake = false;
    m_itemReplaced = false;

    ServerRoulette &serverRoulette = s_serverRoulettes[playerIdx];
    serverRoulette = {};
    serverRoulette.roulette = this;
    serverRoulette.item = ItemId::NoItem;
    serverRoulette.itemHandler = itemHandler;
    serverRoulette.noEarlyStop = m_noEarlyStop;
    m_noEarlyStop = 1;

    ServerItems::request(playerIdx);
}

bool ItemRoulette::calc() {
    s32 playerIdx = findServerRoulette(this);
    if (playerIdx < 0) {
        return REPLACED(calc)();
    }

    ServerRoulette &serverRoulette = s_serverRoulettes[playerIdx];
    if (!serverRoulette.granted) {
        u8 item = ServerItems::NO_ITEM;
        switch (ServerItems::poll(playerIdx, &item)) {
        case ServerItems::Status::Waiting:
            holdSpin();
            break;
        case ServerItems::Status::Granted:
            serverRoulette.granted = true;
            serverRoulette.item = static_cast<ItemId>(item);
            applyServerItem(serverRoulette.item, serverRoulette.itemHandler);
            m_noEarlyStop = serverRoulette.noEarlyStop;
            break;
        default:
            // Refused, timed out, or the request was dropped: no item.
            cancel();
            return false;
        }
    }

    if (serverRoulette.granted) {
        // The game's own item limits and the handshake may replace the item with a Mushroom.
        // The server's decision is final, so undo that before and after every frame.
        m_item = serverRoulette.item;
        m_itemReplaced = false;
    }

    bool landed = REPLACED(calc)();

    if (serverRoulette.granted) {
        m_item = serverRoulette.item;
        m_itemReplaced = false;
        if (landed) {
            // KartItem::calc() gives m_item to the player and resets the roulette.
            releaseServerRoulette(playerIdx);
        }
    }

    return landed;
}

void ItemRoulette::cancel() {
    if (s32 playerIdx = findServerRoulette(this); playerIdx >= 0) {
        releaseServerRoulette(playerIdx);
    }
    REPLACED(cancel)();
}

void ItemRoulette::holdSpin() {
    // calc() increments m_frame and starts stopping once it exceeds m_duration. Keep it below
    // that so the roulette keeps spinning until the server answers.
    if (m_frame >= m_duration) {
        m_frame = m_duration - 1;
    }
}

void ItemRoulette::applyServerItem(ItemId item, Net::ItemHandler *itemHandler) {
    m_item = item;
    m_itemReplaced = false;

    if (itemHandler) {
        // Tell the room about the item through the regular handshake, so other players see the
        // item request they expect. Its outcome is ignored: calc() keeps the server's item.
        itemHandler->requestItem(m_owner->playerId(), static_cast<u32>(item));
        m_waitingForHandshake = true;
    }
}

} // namespace Item
