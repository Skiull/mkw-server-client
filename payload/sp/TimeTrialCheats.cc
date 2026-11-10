#include "TimeTrialCheats.hh"

extern "C" {
#include <revolution.h>
}

extern "C" {
u8 TimeTrialCheats_itemBoxes = 0;
}

namespace SP {

static bool s_practiceUsed = false;

void ResetPracticeUsed() {
    if (s_practiceUsed) {
        SP_LOG("TimeTrialCheats: new race, ghost saving allowed again");
    }
    s_practiceUsed = false;
}

void MarkPracticeUsed() {
    if (!s_practiceUsed) {
        SP_LOG("TimeTrialCheats: practice tool used, this run's ghost won't be saved");
    }
    s_practiceUsed = true;
}

bool PracticeUsed() {
    return s_practiceUsed;
}

} // namespace SP
