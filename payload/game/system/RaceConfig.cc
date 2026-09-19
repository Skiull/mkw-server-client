#include "RaceConfig.hh"

namespace System {

bool RaceConfig::Scenario::isOnline() const {
    return gameMode >= GameMode::OnlinePrivateVS && gameMode <= GameMode::OnlinePrivateBT;
}
RaceConfig::Scenario &RaceConfig::raceScenario() {
    return m_raceScenario;
}

RaceConfig *RaceConfig::Instance() {
    return s_instance;
}

} // namespace System
