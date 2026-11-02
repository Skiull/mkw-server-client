#pragma once

#include "game/ui/ctrl/CtrlMenuBackButton.hh"
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
    void onBackButtonFront(PushButton *button, u32 localPlayerId);
    void refresh();

    template <typename T>
    using H = typename T::template Handler<ReplayListPage>;

    MultiControlInputManager m_inputManager;
    CtrlMenuPageTitleText m_pageTitleText;
    LayoutUIControl m_rows[12];
    CtrlMenuBackButton m_backButton;
    CtrlMenuInstructionText m_instructionText;
    H<MultiControlInputManager> m_onBack{this, &ReplayListPage::onBack};
    H<MultiControlInputManager> m_onFront{this, &ReplayListPage::onFront};
    H<MultiControlInputManager> m_onUp{this, &ReplayListPage::onUp};
    H<MultiControlInputManager> m_onDown{this, &ReplayListPage::onDown};
    H<PushButton> m_onBackButtonFront{this, &ReplayListPage::onBackButtonFront};

    PageId m_replacement = PageId::WifiMenuTop;
    u32 m_cursor = 0;
    u32 m_shownSerial = 0;
    u32 m_framesSinceRequest = 0;
    bool m_loaded = false;
};

} // namespace UI
