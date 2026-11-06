#include "ItemInventory.hh"

#include <sp/TimeTrialCheats.hh>

extern "C" {
#include <revolution.h>
}

namespace Item {

void ItemInventory::resetItem() {
    // Items are kept (not used up) only with the item wheel, in offline Time Trials.
    if (!SP::ItemWheelAllowed()) {
        REPLACED(resetItem)();
    }
}

void ItemInventory::resetHeldItem() {
    // Items are kept (not used up) only with the item wheel, in offline Time Trials.
    if (!SP::ItemWheelAllowed()) {
        REPLACED(resetHeldItem)();
    }
}

ItemId ItemInventory::getCurrentItem() const {
    return m_currentItemID;
}

void ItemInventory::setItemWheelPressed(bool pressed) {
    m_itemWheelPressed = pressed;
}

bool ItemInventory::getItemWheelPressed() const {
    return m_itemWheelPressed;
}

} // namespace Item
