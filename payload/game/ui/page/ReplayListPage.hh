#pragma once

#include "game/ui/ctrl/CtrlMenuInstructionText.hh"
#include "game/ui/ctrl/CtrlMenuPageTitleText.hh"
#include "game/ui/page/Page.hh"

namespace UI {

// The Replays menu: the player's last 12 recorded races (Up/Down to choose, A to watch).
// Watching goes straight to the room search for that replay, without the VS/Battle page.
class ReplayListPage : public Page {
public:
    ReplayListPage();
    ~ReplayListPage() override;

    PageId getReplacement() override;
    void onInit() override;
    void onActivate() override;
    void beforeCalc() override;

private:
    void onBack(u32 localPlayerId);
    void onFront(u32 localPlayerId);
    void onUp(u32 localPlayerId);
    void onDown(u32 localPlayerId);
    void refresh();

    template <typename T>
    using H = typename T::template Handler<ReplayListPage>;

    // No buttons on this page (rows are chosen with Up/Down): a MultiControlInputManager
    // with nothing selected crashes on the first input.
    PageInputManager m_inputManager;
    CtrlMenuPageTitleText m_pageTitleText;
    LayoutUIControl m_rows[12];
    CtrlMenuInstructionText m_instructionText;
    H<PageInputManager> m_onBack{this, &ReplayListPage::onBack};
    H<PageInputManager> m_onFront{this, &ReplayListPage::onFront};
    H<PageInputManager> m_onUp{this, &ReplayListPage::onUp};
    H<PageInputManager> m_onDown{this, &ReplayListPage::onDown};

    PageId m_replacement = PageId::WifiMenuTop;
    u32 m_cursor = 0;
    u32 m_shownSerial = 0;
    u32 m_framesSinceRequest = 0;
    bool m_loaded = false;
};

} // namespace UI
