#pragma once

#include "game/ui/ctrl/CtrlMenuBackButton.hh"
#include "game/ui/ctrl/CtrlMenuInstructionText.hh"
#include "game/ui/ctrl/CtrlMenuPageTitleText.hh"
#include "game/ui/page/Page.hh"

namespace UI {

// MKW Lounge Regular Track ladder, 10 players per page (Left/Right to change page). Opened
// from the online menu's Leaderboard button. Rows come from wfc-server.
class LeaderboardPage : public Page {
public:
    LeaderboardPage();
    ~LeaderboardPage() override;

    PageId getReplacement() override;
    void onInit() override;
    void onActivate() override;
    void beforeCalc() override;

private:
    void onBack(u32 localPlayerId);
    void onLeft(u32 localPlayerId);
    void onRight(u32 localPlayerId);
    void onBackButtonFront(PushButton *button, u32 localPlayerId);
    void request(u16 page);
    void refresh();

    template <typename T>
    using H = typename T::template Handler<LeaderboardPage>;

    MultiControlInputManager m_inputManager;
    CtrlMenuPageTitleText m_pageTitleText;
    LayoutUIControl m_rows[10];
    CtrlMenuBackButton m_backButton;
    CtrlMenuInstructionText m_instructionText;
    H<MultiControlInputManager> m_onBack{this, &LeaderboardPage::onBack};
    H<MultiControlInputManager> m_onLeft{this, &LeaderboardPage::onLeft};
    H<MultiControlInputManager> m_onRight{this, &LeaderboardPage::onRight};
    H<PushButton> m_onBackButtonFront{this, &LeaderboardPage::onBackButtonFront};

    u16 m_page = 0;
    u16 m_pageCount = 1;
    u32 m_shownSerial = 0;
    u32 m_framesSinceRequest = 0;
    char m_shownFlags[10][9] = {}; // Flag pane shown in each row, if any.
};

} // namespace UI
