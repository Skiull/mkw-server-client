#include <Common.S>

// Item boxes in offline Time Trials (TAItemBoxes setting), ported from the henrymkw/mkw-sp fork.
// TimeTrialCheats_itemBoxes is set in RaceScene::createSubsystems, only for offline Time Trials,
// so online races never see a change. r12 is a scratch register here.

// The object manager skips item boxes in Time Trials (r7 = 1): pass 0 instead.
PATCH_B_START(ObjManager_ct, 0x12c)
    li r7, 0x1 // Original instruction
    lis r12, TimeTrialCheats_itemBoxes@ha
    lbz r12, TimeTrialCheats_itemBoxes@l(r12)
    cmpwi r12, 0x0
    beq 1f
    li r7, 0x0
1:
    b ObjManager_ct + 0x130
PATCH_B_END(ObjManager_ct, 0x12c)

// The item roulette gives nothing in Time Trials: act as if it isn't one. At +0x4c/+0x50 the
// game loads the Time Trial flag into r0 (lis r4 / lbz r0, flag@l(r4), filled in when StaticR
// is linked), so the patch goes on the cmpwi after it, which has no relocation. Patching the
// lbz itself with a guessed copy read a random byte online and could block every item.
PATCH_B_START(ItemRoulette_start, 0x54)
    lis r12, TimeTrialCheats_itemBoxes@ha
    lbz r12, TimeTrialCheats_itemBoxes@l(r12)
    cmpwi r12, 0x0
    beq 1f
    li r0, 0x0
1:
    cmpwi r0, 0x0 // Original instruction; the bne after it uses cr0
    b ItemRoulette_start + 0x58
PATCH_B_END(ItemRoulette_start, 0x54)

// Items collide with karts in Time Trials too (upstream nops the early return for Time Trials;
// only Time Trials take it, and they have no items unless a practice tool adds them).
PATCH_REPLACE_START(ItemDirector_checkKartCollision, 0x28)
    nop
    nop
PATCH_REPLACE_END(ItemDirector_checkKartCollision, 0x28)
