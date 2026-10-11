#pragma once

#include <Common.hh>

#include "game/ui/RegisteredPadManager.hh"
#include "game/ui/Section.hh"
#include "game/ui/SectionParams.hh"

namespace UI {

class SectionManager {
public:
    SectionId currentSectionId() {
        return currentSection()->id();
    }

    Section *currentSection() {
        return m_currentSection;
    }

    void REPLACED(createSection)();
    REPLACE void createSection();

    u8 getLocalPlayerCount() const;

    const SectionParams *sectionParams() const {
        return m_sectionParams;
    }

    static SectionManager *Instance() {
        return s_instance;
    }

private:
    Section *m_currentSection;
    u8 _04[0x0c - 0x04];
    SectionId m_nextSectionId;
    SectionId m_lastSectionId;
    u32 m_currentAnimDir;
    u32 m_nextAnimDir;
    u32 m_changeTimer;
    bool m_firstLoad;
    u8 _21[0x2C - 0x21];
    s32 m_transitionFrame;
    u32 m_state;
    RegisteredPadManager m_registeredPadManager;
    u8 _90[0x98 - 0x90];
    SectionParams *m_sectionParams;

    static SectionManager *s_instance;
};
static_assert(sizeof(SectionManager) == 0x9c);

} // namespace UI
