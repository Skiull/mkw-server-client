#include "LeaderboardPage.hh"

#include <nw4r/lyt/lyt_textBox.hh>

#include <sp/net/mkw_server/RoomManager.hh>
#include <sp/net/mkw_server/packets/Leaderboard.hh>

#include <stdio.h>
#include <string.h>

namespace UI {

namespace {

// Rank icon panes in the row layout, in the order of LeaderboardRow::rank.
const char *const RANK_PANES[] = {"rank_iron", "rank_copper", "rank_bronze", "rank_silver",
        "rank_gold", "rank_platinum", "rank_emerald", "rank_ruby", "rank_diamond", "rank_pearl",
        "rank_master", "rank_grandmaster"};

// Messages (CommonSP).
constexpr u32 MSG_TITLE = 10010;
constexpr u32 MSG_STRING = 10011;
constexpr u32 MSG_POSITION = 10012;
constexpr u32 MSG_NUMBER = 10013;
constexpr u32 MSG_PAGE = 10014;
constexpr u32 MSG_LOADING = 10015;

// Ask again if no rows arrived after this many frames (the ladder may still be loading on
// the server).
constexpr u32 RETRY_FRAMES = 120;

// Gives the page access to a row's panes.
class RowControl : public LayoutUIControl {
public:
    nw4r::lyt::Pane *pane(const char *name) {
        return m_mainLayout.findPaneByName(name);
    }
};

// nw4r::lyt::TextBox's top and bottom text colors (RGBA) sit right after its text buffer
// pointer, at 0xdc.
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

LeaderboardPage::LeaderboardPage() = default;

LeaderboardPage::~LeaderboardPage() = default;

PageId LeaderboardPage::getReplacement() {
    return PageId::WifiMenuTop;
}

void LeaderboardPage::onInit() {
    m_inputManager.init(0x1, false);
    setInputManager(&m_inputManager);

    initChildren(3 + std::size(m_rows));
    insertChild(0, &m_pageTitleText, 0);
    insertChild(1, &m_backButton, 0);
    insertChild(2, &m_instructionText, 0);
    for (u32 i = 0; i < std::size(m_rows); i++) {
        insertChild(3 + i, &m_rows[i], 0);
        char variant[0x10];
        snprintf(variant, sizeof(variant), "Row%u", i);
        m_rows[i].load("control", "LeaderboardRow", variant, nullptr);
    }

    m_pageTitleText.load(false);
    m_backButton.load("button", "Back", "ButtonBack", 0x1, false, true);
    m_instructionText.load();

    m_inputManager.setHandler(MenuInputManager::InputId::Back, &m_onBack, false, false);
    m_inputManager.setHandler(MenuInputManager::InputId::Left, &m_onLeft, false, false);
    m_inputManager.setHandler(MenuInputManager::InputId::Right, &m_onRight, false, false);
    m_backButton.setFrontHandler(&m_onBackButtonFront, false);

    m_pageTitleText.setMessage(MSG_TITLE);
}

void LeaderboardPage::onActivate() {
    m_backButton.selectDefault(0);
    request(m_page);
    refresh();
}

void LeaderboardPage::request(u16 page) {
    MKWServer::sendLeaderboardRequest(page);
    m_framesSinceRequest = 0;
}

void LeaderboardPage::beforeCalc() {
    // The room manager connection is only polled during match making otherwise.
    MKWServer::recvFromRoomManager();
    if (MKWServer::leaderboardPageSerial() != m_shownSerial) {
        refresh();
    } else if (++m_framesSinceRequest >= RETRY_FRAMES &&
            MKWServer::latestLeaderboardPage().rows[0].position == 0) {
        request(m_page);
    }
}

void LeaderboardPage::refresh() {
    m_shownSerial = MKWServer::leaderboardPageSerial();
    const auto &page = MKWServer::latestLeaderboardPage();
    bool loaded = page.rows[0].position != 0;
    if (loaded) {
        m_page = page.page;
        m_pageCount = page.pageCount == 0 ? 1 : page.pageCount;
        MessageInfo info{};
        info.intVals[0] = m_page + 1;
        info.intVals[1] = m_pageCount;
        m_instructionText.setMessage(MSG_PAGE, &info);
    } else {
        m_instructionText.setMessage(MSG_LOADING, nullptr);
    }

    for (u32 i = 0; i < std::size(m_rows); i++) {
        auto &control = m_rows[i];
        const auto &row = page.rows[i];
        bool show = loaded && row.position != 0;
        control.setVisible(show);
        if (!show) {
            continue;
        }

        MessageInfo posInfo{};
        posInfo.intVals[0] = row.position;
        control.setMessage("position", MSG_POSITION, &posInfo);

        for (u32 r = 0; r < std::size(RANK_PANES); r++) {
            control.setPaneVisible(RANK_PANES[r], r == row.rank);
        }

        wchar_t name[17] = {};
        memcpy(name, row.name, sizeof(row.name));
        MessageInfo nameInfo{};
        nameInfo.strings[0] = name;
        control.setMessage("name", MSG_STRING, &nameInfo);

        wchar_t country[3] = {static_cast<wchar_t>(row.country[0]),
                static_cast<wchar_t>(row.country[1]), 0};
        MessageInfo countryInfo{};
        countryInfo.strings[0] = country;
        control.setMessage("country", MSG_STRING, &countryInfo);

        MessageInfo lrInfo{};
        lrInfo.intVals[0] = row.lr;
        control.setMessage("lr", MSG_NUMBER, &lrInfo);
        setTextColor(static_cast<RowControl &>(control).pane("lr"), row.color);
    }
}

void LeaderboardPage::onBack(u32 /* localPlayerId */) {
    startReplace(Anim::Prev, 0.0f);
}

void LeaderboardPage::onBackButtonFront(PushButton * /* button */, u32 localPlayerId) {
    onBack(localPlayerId);
}

void LeaderboardPage::onLeft(u32 /* localPlayerId */) {
    request(m_page == 0 ? m_pageCount - 1 : m_page - 1);
}

void LeaderboardPage::onRight(u32 /* localPlayerId */) {
    request(m_page + 1 >= m_pageCount ? 0 : m_page + 1);
}

} // namespace UI
