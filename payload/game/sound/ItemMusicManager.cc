// See LICENSE-mkw-sp

#include "ItemMusicManager.hh"

#include "game/sound/RaceSoundManager.hh"

#include <sp/settings/Settings.hh>

namespace Sound {

ItemMusicManager *ItemMusicManager::CreateInstance() {
    auto *instance = REPLACED(CreateInstance)();
    instance->m_speedup = false;
    return instance;
}

ItemMusicManager *ItemMusicManager::Instance() {
    return s_instance;
}

// Item Music setting.
void ItemMusicManager::resolve() {
    REPLACED(resolve)();

    auto setting = Settings::getSetting<Settings::Setting::ItemMusic>();

    switch (m_state) {
    case State::KinokoBig:
    case State::Star:
        if (setting != Settings::ItemMusic::All) {
            m_state = State::Idle;
        }
        break;
    case State::Damage:
        if (setting == Settings::ItemMusic::None) {
            m_state = State::Idle;
        }
        break;
    default:
        break;
    }
}

// Final Lap Speedup "Dynamic": the music is played a tone higher on the final lap
// (ItemMusicManager.S).
f32 ItemMusicManager::pitch() const {
    u32 state = RaceSoundManager::Instance()->state();
    if (state < 4 || state > 6) {
        return m_pitch;
    }

    if (!m_speedup) {
        return m_pitch;
    }

    return m_pitch * (9.0f / 8.0f); // One major tone
}

} // namespace Sound
