#include <Common.S>

// No Lap Increase (offline Time Trials, sp/TimeTrialCheats.hh): the lap counter goes
// lap = lap + 1 at RaceManagerPlayer_updateLap+0x44 (PAL 0x805349fc, "addi r5, r4, 1"; Vega's
// Lap Count Increased Modifier writes that 1). Add 0 instead while the flag is set.
PATCH_B_START(RaceManagerPlayer_updateLap, 0x44)
    addi r5, r4, 0x1 // Original instruction
    lis r12, TimeTrialCheats_noLapIncrease@ha
    lbz r12, TimeTrialCheats_noLapIncrease@l(r12)
    cmpwi r12, 0x0
    beq 1f
    mr r5, r4
1:
    b RaceManagerPlayer_updateLap + 0x48
PATCH_B_END(RaceManagerPlayer_updateLap, 0x44)
