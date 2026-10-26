#include "ItemHandler.hh"

namespace Net {

static bool s_holdRequests = false;
static ItemHandler *s_heldRequestHandler = nullptr;

void ItemHandler::requestItem(u32 playerId, u32 item) {
    if (s_holdRequests) {
        s_heldRequestHandler = this;
        return;
    }

    REPLACED(requestItem)(playerId, item);
}

void ItemHandler::beginHoldRequests() {
    s_holdRequests = true;
    s_heldRequestHandler = nullptr;
}

ItemHandler *ItemHandler::endHoldRequests() {
    s_holdRequests = false;
    ItemHandler *handler = s_heldRequestHandler;
    s_heldRequestHandler = nullptr;
    return handler;
}

} // namespace Net
