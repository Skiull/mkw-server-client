#include "RaceConfig.hh"

namespace System {

bool RaceConfig::Scenario::isOnline() const {
    return gameMode >= Registry::GameMode::OnlinePrivateVS &&
            gameMode <= Registry::GameMode::OnlinePrivateBT;
}

bool RaceConfig::Scenario::isOnlineVS() const {
    return gameMode >= Registry::GameMode::OnlinePrivateVS &&
            gameMode <= Registry::GameMode::OnlinePublicVS;
}

bool RaceConfig::Scenario::isOnlineBattle() const {
    return gameMode >= Registry::GameMode::OnlinePrivateBT &&
            gameMode <= Registry::GameMode::OnlinePublicBT;
}

RaceConfig::Scenario &RaceConfig::raceScenario() {
    return m_raceScenario;
}

RaceConfig *RaceConfig::Instance() {
    return s_instance;
}

} // namespace System
