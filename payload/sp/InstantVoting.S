#include <Common.S>

// Instant Voting: skip the course roulette's spin but still show the picked course for the
// usual 3 seconds (always on; s_instantVoting in sp/FastMenus.cc).
//
// In the roulette's spin state (VotingPage calc, PAL 0x80643b68), 5800(r28) is the highlighted
// slot, 5804 the chosen one, 5812 the spin speed. Each frame the highlight moves on; once the
// speed drops below a threshold with the highlight on the chosen slot, the game marks it,
// starts the 180 frame timer and moves to the next state. Here, at the "did the highlight
// move" check (r5 = old slot, r0 = new slot; originally beq 0x80643e78), the highlight is put
// on the chosen slot and the speed set to 0, so that happens on this frame.
// (Ro's "Instant Voting Roulette Decide" code, https://mariokartwii.com/showthread.php?tid=2123,
// jumped to the end of the timer instead, so the picked course was never shown.)
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
    lwz r0, 5804 (r28) // The chosen slot
    stw r0, 5800 (r28)
    li r12, 0
    stw r12, 5812 (r28) // Spin speed 0.0f
    cmpw r5, r0
    bne 3f
    // Already highlighted: go straight to marking it.
    mr r3, r0
    b InstantVoting_80643bc4 + 0x7c // 0x80643c40
3:
    // Move the highlight there; the speed and slot checks after it then pass.
    b InstantVoting_80643bc4 + 0x4
PATCH_B_END(InstantVoting_80643bc4, 0x0)
