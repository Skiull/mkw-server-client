// See LICENSE-mkw-sp

#pragma once

#include "game/ui/SectionId.hh"
#include "game/ui/page/Page.hh"

#include "game/host_system/ContextId.hh"
#include "game/host_system/Scene.hh"
#include "game/sound/GroupId.hh"

#include <nw4r/lyt/lyt_drawInfo.hh>

namespace UI {

class WifiRaceEndCountdownPage;
class MenuSettingsPage;
class WifiMenuTopPage;

class Section {
private:
    template <PageId P>
    struct PageIdHelper;

public:
    Section();

    Page *page(PageId pageId);

    template <PageId P>
    PageIdHelper<P>::type *page() {
        return reinterpret_cast<PageIdHelper<P>::type *>(m_pages[static_cast<size_t>(P)]);
    }

    SectionId id() const {
        return m_id;
    }
    bool isPageFocused(const Page *page) const;
    bool isPageActive(PageId pageId) const;
    Vec2<f32> scaleFor() const;

    static System::SceneId GetSceneId(SectionId id);
    static const char *GetResourceName(SectionId id);
    static bool HasBackModel(SectionId id);
    static System::ContextId GetContextId(const SectionId id);

    static Sound::SoundId GetSoundId(const SectionId id);
    static Sound::GroupId GetGroupId(const SectionId id);

    static s32 GetPriority(const SectionId id);
    static s32 GetSoundTrigger(const PageId id);
    void createFriendListManager();

private:
    void REPLACED(addPage)(PageId pageId);
    REPLACE void addPage(PageId pageId);
    void REPLACED(addActivePage)(PageId pageId);
    REPLACE void addActivePage(PageId pageId);
    void REPLACED(addPages)(SectionId id);
    REPLACE void addPages(SectionId id);
    void REPLACED(addActivePages)(SectionId id);
    REPLACE void addActivePages(SectionId id);

    REPLACE Page *createPage(PageId pageId);
    Page *REPLACED(createPage)(PageId pageId);

    static Page *CreatePage(PageId pageId);

public:
    REPLACE void init(SectionId sectionId);
    REPLACE void deinit();

private:
    void REPLACED(init)(SectionId id);
    void REPLACED(deinit)();

    void popActivePages(s32);

    SectionId m_id;
    u8 _004[0x008 - 0x004];
    std::array<Page *, StandardPageCount()> m_pages;
    Page *m_activePages[10];
    u32 m_activePageCount;
    std::array<Page *, 2> m_systemPages;
    u8 _388[0x390 - 0x388];
    nw4r::lyt::DrawInfo m_drawInfo;
    u8 _3e4[0x3f8 - 0x3e4];
    Vec2<f32> m_scaleFor;
    u8 _400[0x408 - 0x400];
    std::array<Page *, ExtendedPageCount()> m_pageExtensions{};
};
static_assert(sizeof(Section) ==
        ROUND_UP(0x408 + std::max(1uz, sizeof(Page *) * ExtendedPageCount()), alignof(Section)));

template <>
struct Section::PageIdHelper<PageId::WifiRaceEndCountDown> {
    using type = WifiRaceEndCountdownPage;
};

template <>
struct Section::PageIdHelper<PageId::WifiMenuTop> {
    using type = WifiMenuTopPage;
};

template <>
struct Section::PageIdHelper<PageId::MenuSettings> {
    using type = MenuSettingsPage;
};

} // namespace UI
