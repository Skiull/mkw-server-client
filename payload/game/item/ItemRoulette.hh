#pragma once

#include <Common.hh>

#include "game/item/ItemInventory.hh"

namespace Kart {
class KartItem;
}

namespace Net {
class ItemHandler;
}

namespace Item {

// The item box roulette. One per KartItem, at KartItem+0x54.
//
// start() rolls the item right away (m_item); calc() spins the roulette and returns true once it
// has landed on m_item, after which KartItem::calc() gives the item to the player.
//
// When connected to mkw-server, the item is decided by the server instead (see
// sp/net/mkw_server/ServerItems.hh): start() asks the server, and calc() keeps the roulette
// spinning until it answers.
class ItemRoulette {
public:
    // 0x807b9fb0
    // Called by KartItem::setItem (0x80798c38) when the player touches an item box. Rolls the
    // item, applies the item limits (offline) or starts the item handshake with the room
    // (online). r4 is the item box type, r5 the player's position.
    REPLACE void start(u32 r4, u32 r5, u32 r6, u32 r7);
    void REPLACED(start)(u32 r4, u32 r5, u32 r6, u32 r7);

    // 0x807ba37c
    // Advances the roulette by a frame. Returns true when it has landed on m_item.
    REPLACE bool calc();
    bool REPLACED(calc)();

    // 0x807ba5d8
    // Stops the roulette without giving an item and withdraws the online request.
    REPLACE void cancel();
    void REPLACED(cancel)();

private:
    enum class State : u32 {
        Idle = 0,
        Spinning = 1,
        Stopping = 2,
    };

    // Called every frame while the server hasn't answered yet.
    void holdSpin();
    // Called once the server has answered with an item.
    void applyServerItem(ItemId item, Net::ItemHandler *itemHandler);

    Kart::KartItem *m_owner;
    State m_state;
    s32 m_frame;
    // Frames to spin before stopping.
    s32 m_duration;
    f32 m_speed;
    f32 m_scroll;
    ItemId m_prevDisplayItem;
    ItemId m_displayItem;
    // The item the roulette lands on.
    ItemId m_item;
    ItemId _24;
    u16 m_itemBoxType;
    u8 m_position;
    u8 _2b;
    // Set to prevent the player from stopping the roulette early with the item button.
    u8 m_noEarlyStop;
    // Online: waiting for the room to answer the item handshake.
    bool m_waitingForHandshake;
    // Set when the item limits or the handshake replaced m_item with a Mushroom.
    bool m_itemReplaced;
    u8 _2f;
    u32 _30;
};
static_assert(sizeof(ItemRoulette) == 0x34);

} // namespace Item
