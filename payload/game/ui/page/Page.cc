// See LICENSE-mkw-sp

#include "Page.hh"

#include "game/system/SaveManager.hh"
#include "game/ui/SectionManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>

namespace UI {

Page::~Page() = default;

void Page::dt(s32 type) {
    if (type > 0) {
        delete this;
    } else {
        this->~Page();
    }
}

Page::State Page::state() const {
    return m_state;
}

Page::Anim Page::anim() const {
    return m_anim;
}

const MenuInputManager *Page::inputManager() const {
    return m_inputManager;
}

MenuInputManager *Page::inputManager() {
    return m_inputManager;
}

// DriftSelectPage::m_replacementSection (mkw-sp DriftSelectPage.hh): where the online flow
// goes once the player has picked a character, vehicle and drift type.
static constexpr u32 DRIFT_SELECT_REPLACEMENT_SECTION = 0x6c8;

// Watching a replay: instead of character, vehicle and drift select, go straight where the
// drift select page would go (the race being watched), keeping the last picks.
static void trySkipCharacterSelect(Page *page) {
    if (!MKWServer::s_skipCharacterSelect || page->id() != PageId::CharacterSelect) {
        return;
    }
    MKWServer::s_skipCharacterSelect = false;
    auto *driftSelect = SectionManager::Instance()->currentSection()->page(PageId::DriftSelect);
    if (driftSelect == nullptr) {
        SP_LOG("Replay: no drift select page, can't skip character select");
        return;
    }
    SectionId next = *reinterpret_cast<SectionId *>(
            reinterpret_cast<u8 *>(driftSelect) + DRIFT_SELECT_REPLACEMENT_SECTION);
    SP_LOG("Replay: skipping character select, next section %d", static_cast<s32>(next));
    if (next != SectionId::None) {
        page->changeSection(next, Page::Anim::Next, 0.0f);
    }
}

void Page::calc() {
    if (m_state == State::State4) {
        trySkipCharacterSelect(this);
    }
    switch (m_state) {
    case State::State3:
    case State::State4:
    case State::State5:
        beforeCalc();
        m_controlGroup.calc();
        afterCalc();
        calcAnim();
        break;
    default:
        break;
    }
}

} // namespace UI
