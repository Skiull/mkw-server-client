#include "UIControl.hh"

#include "game/ui/ControlLoader.hh"

#include <sp/net/mkw_server/packets/LoungeRanks.hh>

#include <string.h>

namespace UI {

UIControl::~UIControl() = default;

void UIControl::dt(s32 type) {
    if (type > 0) {
        delete this;
    } else {
        this->~UIControl();
    }
}

void UIControl::setVisible(bool visible) {
    m_isHidden = !visible;
}

bool UIControl::getVisible() const {
    return !m_isHidden;
}

const Page *UIControl::getPage() const {
    return m_controlGroup->m_page;
}

Page *UIControl::getPage() {
    return m_controlGroup->m_page;
}

void LayoutUIControl::setMessage(const char *pane, u32 messageId, MessageInfo *info) {
    REPLACED(setMessage)(pane, messageId, info);
    // Check Members' VR number ("{n}"), which shows the LR in mogi rooms.
    if (messageId == 5105 && info != nullptr && pane != nullptr && strcmp(pane, "point") == 0) {
        MKWServer::colorPaneByLR(m_mainLayout.findPaneByName(pane), info->intVals[0]);
    }
}

LayoutUIControl::LayoutUIControl() = default;

LayoutUIControl::~LayoutUIControl() {
    m_commonMessageGroup.dt(-1);
    m_specificMessageGroup.dt(-1);
    m_mainLayout.dt(-1);
    m_animator.dt(-1);
}

void LayoutUIControl::load(const char *dir, const char *file, const char *variant,
        const char *const *groups) {
    ControlLoader loader(this);
    loader.load(dir, file, variant, groups);
}

LayoutUIControlScaleFade::LayoutUIControlScaleFade() = default;

LayoutUIControlScaleFade::~LayoutUIControlScaleFade() = default;

extern "C" {
void *ControlCheckValid(void *resource, const char *path) {
    if (resource) {
#if LOG_CONTROL_LOAD
        SP_LOG("Control loaded: %s", path);
#endif
        return resource;
    }

    panic("Failed to load '%s'!", path);
}
}

} // namespace UI
