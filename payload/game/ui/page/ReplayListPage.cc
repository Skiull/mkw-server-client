#include "ReplayListPage.hh"

#include "game/ui/SectionManager.hh"

#include <nw4r/lyt/lyt_textBox.hh>
#include "game/ui/page/RandomMatchingPage.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/net/mkw_server/RoomManager.hh>
#include <sp/net/mkw_server/packets/ReplayList.hh>

#include <stdio.h>
#include <string.h>

namespace UI {

namespace {

// Messages (CommonSP).
constexpr u32 MSG_TITLE = 10008;   // "Replays"
constexpr u32 MSG_STRING = 10011;  // "{arg string|0}"
constexpr u32 MSG_POSITION = 10012; // "{arg integer|0 0}."
constexpr u32 MSG_HELP = 10017;
constexpr u32 MSG_LOADING = 10018;
constexpr u32 MSG_EMPTY = 10019;

constexpr u32 RETRY_FRAMES = 120;

const u8 WHITE[3] = {255, 255, 255};
const u8 YELLOW[3] = {255, 225, 0};

// Gives the page access to a row's panes.
class RowControl : public LayoutUIControl {
public:
    nw4r::lyt::Pane *pane(const char *name) {
        return m_mainLayout.findPaneByName(name);
    }
};

// nw4r::lyt::TextBox's top and bottom text colors (RGBA), at 0xdc.
void setTextColor(nw4r::lyt::Pane *pane, const u8 *rgb) {
    if (!pane) {
        return;
    }
    u8 *colors = reinterpret_cast<u8 *>(pane) + 0xdc;
    for (u32 i = 0; i < 2; i++) {
        colors[i * 4 + 0] = rgb[0];
        colors[i * 4 + 1] = rgb[1];
        colors[i * 4 + 2] = rgb[2];
        colors[i * 4 + 3] = 255;
    }
}

} // namespace

ReplayListPage::ReplayListPage() = default;

ReplayListPage::~ReplayListPage() = default;

PageId ReplayListPage::getReplacement() {
    return m_replacement;
}

void ReplayListPage::onInit() {
    m_inputManager.init(0x1, false);
    setInputManager(&m_inputManager);

    initChildren(2 + std::size(m_rows));
    insertChild(0, &m_pageTitleText, 0);
    insertChild(1, &m_instructionText, 0);
    for (u32 i = 0; i < std::size(m_rows); i++) {
        insertChild(2 + i, &m_rows[i], 0);
        char variant[0x10];
        snprintf(variant, sizeof(variant), "Replay%u", i);
        m_rows[i].load("globe", "LeaderboardRow", variant, nullptr);
    }

    m_pageTitleText.load(false);
    m_instructionText.load();

    m_inputManager.setHandler(MenuInputManager::InputId::Back, &m_onBack, false);
    m_inputManager.setHandler(MenuInputManager::InputId::Front, &m_onFront, false);
    m_inputManager.setHandler(MenuInputManager::InputId::Up, &m_onUp, true);
    m_inputManager.setHandler(MenuInputManager::InputId::Down, &m_onDown, true);

    m_pageTitleText.setMessage(MSG_TITLE);
}

void ReplayListPage::onActivate() {
    m_replacement = PageId::WifiMenuTop;
    m_cursor = 0;
    m_loaded = false;
    MKWServer::sendReplayListRequest();
    m_framesSinceRequest = 0;
    m_shownSerial = MKWServer::replayListSerial();
    refresh();
}

void ReplayListPage::beforeCalc() {
    // The room manager connection is only polled during match making otherwise.
    MKWServer::recvFromRoomManager();
    if (MKWServer::replayListSerial() != m_shownSerial) {
        m_shownSerial = MKWServer::replayListSerial();
        m_loaded = true;
        refresh();
    } else if (!m_loaded && ++m_framesSinceRequest >= RETRY_FRAMES) {
        MKWServer::sendReplayListRequest();
        m_framesSinceRequest = 0;
    }
}

void ReplayListPage::refresh() {
    const auto &list = MKWServer::latestReplayList();
    u32 count = m_loaded ? list.count : 0;
    if (count > std::size(m_rows)) {
        count = std::size(m_rows);
    }
    if (m_cursor >= count) {
        m_cursor = count == 0 ? 0 : count - 1;
    }
    m_instructionText.setMessage(!m_loaded ? MSG_LOADING : count == 0 ? MSG_EMPTY : MSG_HELP,
            nullptr);

    for (u32 i = 0; i < std::size(m_rows); i++) {
        auto &control = m_rows[i];
        control.setVisible(i < count);
        if (i >= count) {
            continue;
        }
        const auto &e = list.entries[i];

        MessageInfo posInfo{};
        posInfo.intVals[0] = i + 1;
        control.setMessage("position", MSG_POSITION, &posInfo);

        wchar_t course[25] = {};
        memcpy(course, e.course, sizeof(e.course));
        MessageInfo courseInfo{};
        courseInfo.strings[0] = course;
        control.setMessage("name", MSG_STRING, &courseInfo);

        wchar_t when[25] = {};
        memcpy(when, e.when, sizeof(e.when));
        MessageInfo whenInfo{};
        whenInfo.strings[0] = when;
        control.setMessage("lr", MSG_STRING, &whenInfo);

        const u8 *color = i == m_cursor ? YELLOW : WHITE;
        auto &row = static_cast<RowControl &>(control);
        setTextColor(row.pane("position"), color);
        setTextColor(row.pane("name"), color);
        setTextColor(row.pane("lr"), color);
    }
}

void ReplayListPage::onUp(u32 /* localPlayerId */) {
    u32 count = m_loaded ? MKWServer::latestReplayList().count : 0;
    if (count == 0) {
        return;
    }
    m_cursor = m_cursor == 0 ? count - 1 : m_cursor - 1;
    refresh();
}

void ReplayListPage::onDown(u32 /* localPlayerId */) {
    u32 count = m_loaded ? MKWServer::latestReplayList().count : 0;
    if (count == 0) {
        return;
    }
    m_cursor = m_cursor + 1 >= count ? 0 : m_cursor + 1;
    refresh();
}

void ReplayListPage::onFront(u32 /* localPlayerId */) {
    u32 count = m_loaded ? MKWServer::latestReplayList().count : 0;
    if (m_cursor >= count) {
        return;
    }
    // Straight to the room search: a VS room in the replay region, for this replay.
    auto *section = SectionManager::Instance()->currentSection();
    auto *randomMatchingPage =
            static_cast<RandomMatchingPage *>(section->page(PageId::WifiRandomMatching));
    if (randomMatchingPage == nullptr) {
        return;
    }
    MKWServer::s_watchReplay = true;
    MKWServer::s_replayIndex = m_cursor;
    MKWServer::s_skipCharacterSelect = true;
    randomMatchingPage->setJoinType(RandomMatchingPage::JoinType::AnyoneRegional);
    randomMatchingPage->setBattle(false);
    m_replacement = PageId::WifiRandomMatching;
    startReplace(Anim::Next, 0.0f);
}

void ReplayListPage::onBack(u32 /* localPlayerId */) {
    m_replacement = PageId::WifiMenuTop;
    startReplace(Anim::Prev, 0.0f);
}

} // namespace UI
