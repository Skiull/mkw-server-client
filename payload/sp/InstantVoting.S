#include <Common.S>

// Instant Voting: Ro's "Instant Voting Roulette Decide / Skip Voting" code
// (https://mariokartwii.com/showthread.php?tid=2123) as a setting. The code turns this beq into
// b +0x11c (straight to the race once the host decides the course); here the hook takes that
// jump only while s_instantVoting is set (sp/FastMenus.cc), so the patch itself, and the game
// hash, are the same either way. cr0 still holds the original compare.
PATCH_B_START(InstantVoting_80643bc4, 0x0)
    lis r12, s_instantVoting@ha
    lbz r12, s_instantVoting@l (r12)
    cmpwi cr1, r12, 0
    bne cr1, 1f
    bne 2f
    b InstantVoting_80643bc4 + 0x2b4 // Original: beq 0x80643e78
2:
    b InstantVoting_80643bc4 + 0x4
1:
    b InstantVoting_80643bc4 + 0x11c
PATCH_B_END(InstantVoting_80643bc4, 0x0)
