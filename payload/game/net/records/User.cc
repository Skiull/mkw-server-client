#include "User.hh"

#include "game/system/RaceConfig.hh"

namespace Net {

const UserRecord *UserHandler::outgoingRecord() const {
    createUser();

    return &m_recordToSend;
}

bool UserHandler::prepared() {
    return m_prepared;
}

u16 UserHandler::getRating() const {
    auto &raceScenario = System::RaceConfig::Instance()->raceScenario();

    if (raceScenario.isOnlineVS()) {
        return myVR();
    }
    if (raceScenario.isOnlineBattle()) {
        return myBR();
    }

    return 5000;
}

u16 UserHandler::myVR() const {
    return m_recordToSend.vr;
}

u16 UserHandler::myBR() const {
    return m_recordToSend.br;
}

} // namespace Net
