#pragma once

#include <Common.hh>

#include "game/ui/ctrl/CtrlMenuBackButton.hh"
#include "game/ui/ctrl/CtrlMenuInstructionText.hh"
#include "game/ui/ctrl/CtrlMenuPageTitleText.hh"
#include "game/ui/ctrl/FriendButton.hh"
#include "game/ui/page/RandomMatchingPage.hh"

namespace UI {

class WifiMenuTopPage : public Page {
public:
    WifiMenuTopPage();
    ~WifiMenuTopPage();

    void onInit() override;
    void onActivate() override;
    void beforeCalc() override;

    // Needed or the game will silently not know where to transition.
    PageId getReplacement() override;

private:
    void onBack(u32 localPlayerId);
    void onButtonSelect(PushButton *button, u32 localPlayerId);
    void onWorldWideButtonFront(PushButton *button, u32 localPlayerId);
    void onRegionalButtonFront(PushButton *button, u32 localPlayerId);
    void onFriendButtonFront(PushButton *button, u32 localPlayerId);
    void onReplaysButtonFront(PushButton *button, u32 localPlayerId);
    void onLeaderboardButtonFront(PushButton *button, u32 localPlayerId);
    void onBackButtonFront(PushButton *button, u32 localPlayerId);
    void onSettingsButtonFront(PushButton *button, u32 localPlayerId);

    s32 getTitleMessageId();
    void transitionToPublicRoomModeSelect(PushButton *button,
            RandomMatchingPage::JoinType joinType);

    template <typename T>
    using H = typename T::template Handler<WifiMenuTopPage>;

    H<PushButton> m_onWorldWideButtonFront{this, &WifiMenuTopPage::onWorldWideButtonFront};
    H<PushButton> m_onRegionalButtonFront{this, &WifiMenuTopPage::onRegionalButtonFront};
    H<PushButton> m_onFriendButtonFront{this, &WifiMenuTopPage::onFriendButtonFront};
    H<PushButton> m_onReplaysButtonFront{this, &WifiMenuTopPage::onReplaysButtonFront};
    H<PushButton> m_onLeaderboardButtonFront{this, &WifiMenuTopPage::onLeaderboardButtonFront};
    H<PushButton> m_onBackButtonFront{this, &WifiMenuTopPage::onBackButtonFront};
    H<PushButton> m_onButtonSelect{this, &WifiMenuTopPage::onButtonSelect};
    H<MultiControlInputManager> m_onBack{this, &WifiMenuTopPage::onBack};

    CtrlMenuPageTitleText m_pageTitleText;
    PushButton m_worldWideButton;
    PushButton m_regionalButton;
    FriendButton m_friendButton;
    CtrlMenuBackButton m_backButton;
    CtrlMenuInstructionText m_instructionText;
    MultiControlInputManager m_inputManager;
    PageId m_replacement;

    PushButton m_settingsButton;
    PushButton m_replaysButton;
    // Mogi button pressed: waiting for the server to say whether the player is linked to
    // Lounge, and the button to continue with.
    bool m_mogiPending = false;
    u32 m_mogiSerial = 0;
    u32 m_mogiFrames = 0;
    PushButton *m_mogiButton = nullptr;
    PushButton m_leaderboardButton;
    H<PushButton> m_onSettingsButtonFront{this, &WifiMenuTopPage::onSettingsButtonFront};
};

} // namespace UI
