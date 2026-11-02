// See LICENSE-mkw-sp

#include "Section.hh"

#include "game/ui/page/LeaderboardPage.hh"
#include "game/ui/page/ReplayListPage.hh"
#include "game/ui/page/SettingsPage.hh"
#include "game/ui/page/WifiMenuTopPage.hh"

namespace UI {

Page *Section::page(PageId pageId) {
    return m_pages[static_cast<size_t>(pageId)];
}

bool Section::isPageActive(PageId pageId) const {
    for (u32 i = 0; i < m_activePageCount; i++) {
        if (m_activePages[i]->id() == pageId) {
            return true;
        }
    }
    return false;
}

Vec2<f32> Section::scaleFor() const {
    return m_scaleFor;
}

void Section::addPage(PageId pageId) {
    std::pair<SectionId, PageId> deletions[] = {

    };
    for (const auto &deletion : deletions) {
        if (deletion.first == m_id && deletion.second == pageId) {
            return;
        }
    }

    REPLACED(addPage)(pageId);
}

void Section::addActivePage(PageId pageId) {
    std::pair<SectionId, PageId> deletions[] = {

    };
    for (const auto &deletion : deletions) {
        if (deletion.first == m_id && deletion.second == pageId) {
            return;
        }
    }

    REPLACED(addActivePage)(pageId);
}

void Section::addPages(SectionId id) {
    REPLACED(addPages)(id);

    std::pair<SectionId, PageId> additions[] = {
            // clang-format off
            {SectionId::WifiSingle, PageId::MenuSettings},
            {SectionId::WifiMulti, PageId::MenuSettings},
            {SectionId::WifiSingle, PageId::MenuLeaderboard},
            {SectionId::WifiMulti, PageId::MenuLeaderboard},
            {SectionId::WifiSingle, PageId::MenuReplayList},
            {SectionId::WifiMulti, PageId::MenuReplayList},
            // clang-format on
    };
    for (const auto &addition : additions) {
        if (addition.first == id) {
            addPage(addition.second);
        }
    }
}

void Section::addActivePages(SectionId id) {
    REPLACED(addActivePages)(id);

    std::pair<SectionId, PageId> additions[] = {

    };
    for (const auto &addition : additions) {
        if (addition.first == id) {
            addActivePage(addition.second);
        }
    }
}

Page *Section::createPage(PageId pageId) {
    switch (pageId) {
    case PageId::WifiMenuTop:
        return new WifiMenuTopPage;
    case PageId::MenuSettings:
        return new MenuSettingsPage;
    case PageId::MenuLeaderboard:
        return new LeaderboardPage;
    case PageId::MenuReplayList:
        return new ReplayListPage;
    default:
        return REPLACED(createPage)(pageId);
    }
}

void Section::init(SectionId sectionId) {
    m_pageExtensions = {};
    REPLACED(init)(sectionId);
}

void Section::deinit() {
    // Since we zero out the arrays, we can trust REPLACED(deinit) does not double-free.
    popActivePages(0);
    for (auto *&page : m_pages) {
        if (page == nullptr) {
            continue;
        }
        page->onDeinit();
        // `delete page` would call the *GCC* dtor--which is almost always null!
        page->dt(1);
        page = nullptr;
    }
    for (auto *&page : m_pageExtensions) {
        if (page == nullptr) {
            continue;
        }
        page->onDeinit();
        // `delete page` would call the *GCC* dtor--which is almost always null!
        page->dt(1);
        page = nullptr;
    }
    // We assume it is safe to call SectionInputThing_deinitMaybe even after pages have been
    // deinialized, although usually it is called first.
    REPLACED(deinit)();
}

} // namespace UI
