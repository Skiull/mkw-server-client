#include "FastMenus.hh"

#include "game/ui/SectionManager.hh"

#include <sp/net/mkw_server/GameHash.hh>
#include <sp/settings/Settings.hh>

// Fast Menu Transitions: east's "Faster Menu Navigation" code
// (https://mariokartwii.com/showthread.php?tid=389), toggleable:
// - li r0, 0 at PAL 0x80635670, which zeroes SectionManager's change timer: done here on the
//   timer each frame instead of patching code.
// - Two float constants set to 0 (640.0 in main.dol .sdata2, 176.0 in StaticR.rel .rodata),
//   which drop the pages' slide-in. The StaticR one is hashed as its original value, so the
//   game hash is the same with the setting on or off.
// Read by the Instant Voting hook (sp/InstantVoting.S).
extern "C" {
u8 s_instantVoting = 0;
// Read by the Far Player Tags hooks (sp/FarPlayerTags.S).
u8 s_farPlayerTags = 0;
extern const f32 s_farTagDistance = 1.0e9f;
}

namespace SP::FastMenus {

namespace {

struct Constants {
    u32 dol, rel;
};

// From the code, per region (game id's 4th letter at 0x80000003).
Constants constantsForRegion() {
    switch (*reinterpret_cast<const char *>(0x80000003)) {
    case 'P':
        return {0x8038900C, 0x80896830};
    case 'E':
        return {0x80384C84, 0x808901B8};
    case 'J':
        return {0x8038898C, 0x80895E80};
    case 'K':
        return {0x8037702C, 0x80885158};
    default:
        return {0, 0};
    }
}

bool s_init = false;
u32 *s_dol = nullptr, *s_rel = nullptr;
u32 s_dolOriginal = 0, s_relOriginal = 0;

} // namespace

void Calc() {
    if (!s_init) {
        s_init = true;
        Constants c = constantsForRegion();
        // Only if they hold what the code expects (640.0f and 176.0f), so a wrong address is
        // never written.
        if (c.dol != 0 && *reinterpret_cast<u32 *>(c.dol) == 0x44200000 &&
                *reinterpret_cast<u32 *>(c.rel) == 0x43300000) {
            s_dol = reinterpret_cast<u32 *>(c.dol);
            s_rel = reinterpret_cast<u32 *>(c.rel);
            s_dolOriginal = *s_dol;
            s_relOriginal = *s_rel;
            MKWServer::GameHash::HashAsOriginal(s_rel, s_relOriginal);
        }
    }

    s_instantVoting = Settings::getSetting<Settings::Setting::InstantVoting>() ==
            Settings::InstantVoting::Enabled;

    s_farPlayerTags = Settings::getSetting<Settings::Setting::FarPlayerTags>() ==
            Settings::FarPlayerTags::Enable;

    bool fast = Settings::getSetting<Settings::Setting::FastMenuTransitions>() ==
            Settings::FastMenuTransitions::Enabled;
    if (s_dol) {
        *s_dol = fast ? 0 : s_dolOriginal;
        *s_rel = fast ? 0 : s_relOriginal;
    }
    if (fast) {
        if (auto *sectionManager = UI::SectionManager::Instance()) {
            if (sectionManager->changeTimer() > 1) {
                sectionManager->changeTimer() = 1;
            }
        }
    }
}

} // namespace SP::FastMenus
