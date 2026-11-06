// See LICENSE-mkw-sp

#include "SoundArchivePlayer.hh"

#include <game/sound/PlayerId.hh>
#include <sp/settings/Settings.hh>

namespace nw4r::snd {

// Volume and Music Volume settings.
void SoundArchivePlayer::update() {
    u32 volume = Settings::getSetting<Settings::Setting::Volume>();
    for (u32 playerId = 0; playerId < m_playerCount; playerId++) {
        m_players[playerId].m_mainOutVolume = volume / 10.0f;
    }
    u32 musicVolume = Settings::getSetting<Settings::Setting::MusicVolume>();
    m_players[static_cast<u32>(Sound::PlayerId::PL_BGM)].m_mainOutVolume *= musicVolume / 10.0f;

    REPLACED(update)();
}

} // namespace nw4r::snd
