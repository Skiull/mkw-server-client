#include "WifiMenuTopPage.hh"

#include "game/ui/SectionManager.hh"
#include "game/ui/page/SettingsPage.hh"

#include <sp/net/mkw_server/MKWServer.hh>

namespace UI {

enum ButtonId {
    WorldWide,
    Regional,
    Friend,
    Settings,
    Back,
    Replays,
    Leaderboard,
};

WifiMenuTopPage::WifiMenuTopPage() = default;

WifiMenuTopPage::~WifiMenuTopPage() = default;

static const char *animInfo[] = {"State", "Offline", "RandomMatching", "FriendParent", nullptr,
        "State2", "Offline2", "RandomMatching2", "FriendParent2", nullptr, nullptr};

s32 WifiMenuTopPage::getTitleMessageId() {
    auto sectionId = SectionManager::Instance()->currentSectionId();
    switch (sectionId) {
    case SectionId::WifiSingle:
    case SectionId::WifiSingleDC:
    case SectionId::WifiSingleFriend:
        return 0x7f1;
        break;
    case SectionId::WifiMulti:
    case SectionId::WifiMultiDC:
    case SectionId::WifiMultiFriend:
        return 0x7f2;
        break;
    default:
        panic("Wrong SectionId!");
        return 0;
    }
}

void WifiMenuTopPage::onInit() {
    m_inputManager.init(1, false);
    setInputManager(&m_inputManager);
    m_inputManager.setWrappingMode(UI::MultiControlInputManager::WrappingMode::XY);
    m_inputManager.setHandler(MenuInputManager::InputId::Back, &m_onBack, false);

    initChildren(10);

    insertChild(0, &m_pageTitleText, 0);
    m_pageTitleText.load(0);
    m_pageTitleText.setMessage(getTitleMessageId());

    insertChild(1, &m_worldWideButton, 0);
    m_worldWideButton.load("button", "WifiMenuSingleTop", "ButtonWorld", 1, 0, false);
    m_worldWideButton.m_index = ButtonId::WorldWide;
    m_worldWideButton.setFrontHandler(&m_onWorldWideButtonFront, false);
    m_worldWideButton.setSelectHandler(&m_onButtonSelect, false);

    insertChild(2, &m_regionalButton, 0);
    m_regionalButton.load("button", "WifiMenuSingleTop", "ButtonRegion", 1, 0, false);
    m_regionalButton.m_index = ButtonId::Regional;
    m_regionalButton.setFrontHandler(&m_onRegionalButtonFront, false);
    m_regionalButton.setSelectHandler(&m_onButtonSelect, false);

    insertChild(3, &m_friendButton, 0);
    m_friendButton.load(animInfo, "button", "WifiMenuSingleTopFriendButton", "ButtonFriend", 1, 0);

    m_friendButton.setFrontHandler(&m_onFriendButtonFront, false);
    m_friendButton.setSelectHandler(&m_onButtonSelect, false);
    m_friendButton.m_animator.getGroup(4)->setAnimation(0, 0.0);
    m_friendButton.m_animator.getGroup(5)->setAnimation(0, 0.0);
    m_friendButton.m_index = ButtonId::Friend;

    // Below Friends: watch a recorded race (a replay room on the self-hosted server).
    insertChild(8, &m_replaysButton, 0);
    m_replaysButton.load("button", "ReplaysButton", "ButtonReplays", 1, 0, false);
    m_replaysButton.m_index = ButtonId::Replays;
    m_replaysButton.setFrontHandler(&m_onReplaysButtonFront, false);
    m_replaysButton.setSelectHandler(&m_onButtonSelect, false);

    // Bottom right: the MKW Lounge leaderboard.
    insertChild(9, &m_leaderboardButton, 0);
    m_leaderboardButton.load("button", "LeaderboardButton", "Leaderboard", 1, false, false);
    m_leaderboardButton.m_index = ButtonId::Leaderboard;
    m_leaderboardButton.setFrontHandler(&m_onLeaderboardButtonFront, false);
    m_leaderboardButton.setSelectHandler(&m_onButtonSelect, false);

    insertChild(4, &m_settingsButton, 0);
    m_settingsButton.load("button", "SettingsButton", "Option", 1, false, false);
    m_settingsButton.setFrontHandler(&m_onSettingsButtonFront, false);
    m_settingsButton.setSelectHandler(&m_onButtonSelect, false);
    m_settingsButton.m_index = ButtonId::Settings;

    insertChild(5, &m_backButton, 0);
    m_backButton.load("button", "Back", "ButtonBack", 1, 0, false);
    m_backButton.setFrontHandler(&m_onBackButtonFront, false);
    m_backButton.setSelectHandler(&m_onButtonSelect, false);
    m_backButton.m_index = ButtonId::Back;

    insertChild(6, &m_pageTitleText, 0);
    m_pageTitleText.load(false);

    insertChild(7, &m_instructionText, 0);
    m_instructionText.load();

    SectionManager::Instance()->currentSection()->createFriendListManager();
    setAnimSfxIds(Sound::SoundId::SE_DUMMY, Sound::SoundId::SE_DUMMY);
}

void WifiMenuTopPage::onActivate() {
    if (anim() == Anim::Next) {
        m_worldWideButton.selectDefault(0);
        m_instructionText.setMessage(0x10d6, 0);
    }
    m_replacement = PageId::None;
}

PageId WifiMenuTopPage::getReplacement() {
    return m_replacement;
}

void WifiMenuTopPage::onBack(u32 /* localPlayerId */) {
    m_replacement = PageId::None;
    push(PageId::WifiShutdown, Anim::Next);
}

void WifiMenuTopPage::transitionToPublicRoomModeSelect(PushButton *button,
        RandomMatchingPage::JoinType joinType) {
    // TODO: Use Page::downcast(). This requires an ABI break fix.
    auto *section = SectionManager::Instance()->currentSection();
    auto *page = section->page(PageId::WifiRandomMatching);
    auto *randomMatchingPage = static_cast<RandomMatchingPage *>(page);

    if (randomMatchingPage != nullptr) {
        randomMatchingPage->setJoinType(joinType);
        m_replacement = PageId::WifiMenuModeSelect;
        startReplace(Anim::Next, button->getDelay());
    }
}

void WifiMenuTopPage::onWorldWideButtonFront(PushButton *button, u32 /* localPlayerId */) {
    MKWServer::s_watchReplay = false;
    transitionToPublicRoomModeSelect(button, RandomMatchingPage::JoinType::AnyoneWW);
}

void WifiMenuTopPage::onRegionalButtonFront(PushButton *button, u32 /* localPlayerId */) {
    MKWServer::s_watchReplay = false;
    transitionToPublicRoomModeSelect(button, RandomMatchingPage::JoinType::AnyoneRegional);
}

void WifiMenuTopPage::onFriendButtonFront(PushButton *button, u32 /* localPlayerId */) {
    MKWServer::s_watchReplay = false;
    m_replacement = PageId::WifiFriendMenu;
    startReplace(Anim::None, button->getDelay());
}

void WifiMenuTopPage::onReplaysButtonFront(PushButton *button, u32 /* localPlayerId */) {
    // A replay room is a VS room in its own search region. The player races as normal while
    // the recorded racers drive around them.
    MKWServer::s_watchReplay = true;
    transitionToPublicRoomModeSelect(button, RandomMatchingPage::JoinType::AnyoneRegional);
}

void WifiMenuTopPage::onLeaderboardButtonFront(PushButton *button, u32 /* localPlayerId */) {
    m_replacement = PageId::MenuLeaderboard;
    startReplace(Anim::Next, button->getDelay());
}

void WifiMenuTopPage::onBackButtonFront(PushButton * /* button */, u32 localPlayerId) {
    onBack(localPlayerId);
}

void WifiMenuTopPage::onButtonSelect(PushButton *button, u32 /* localPlayerId */) {
    switch (button->m_index) {
    case ButtonId::WorldWide:
        m_instructionText.setMessage(0x10d6, 0);
        break;
    case ButtonId::Regional:
        m_instructionText.setMessage(0x10d7, 0);
        break;
    case ButtonId::Friend:
        m_instructionText.setMessage(0x10d8, 0);
        break;
    case ButtonId::Settings:
        m_instructionText.setMessage(10001, 0);
        break;
    case ButtonId::Leaderboard:
        m_instructionText.setMessage(10016, 0);
        break;
    case ButtonId::Replays:
        m_instructionText.setMessage(10009, 0);
        break;
    case ButtonId::Back:
        // Set to empty string
        m_instructionText.setMessage(5600, 0);
    }
}

void WifiMenuTopPage::onSettingsButtonFront(PushButton *button, u32 /* localPlayerId */) {
    auto *section = SectionManager::Instance()->currentSection();
    auto *settingsPage = section->page<PageId::MenuSettings>();
    settingsPage->configure(nullptr, id());

    m_replacement = PageId::MenuSettings;
    startReplace(Anim::Next, button->getDelay());
}

} // namespace UI
