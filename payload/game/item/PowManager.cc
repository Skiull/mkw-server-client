#include "PowManager.hh"

namespace Item {

PowManager *PowManager::s_current = nullptr;

void PowManager::calc() {
    s_current = this;
    REPLACED(calc)();
}

} // namespace Item
