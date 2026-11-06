#include <Common.S>

// Far Player Tags: the name tag code hides a racer's tag past a distance it loads from
// StaticR .data (0x808D3E14, mkw-sp's playerTagRenderDistance). mkw-sp writes a bigger value
// there; that would show up in the server's data check, so instead these two loads
// (lfs f0, 0 (r24)) read a huge distance while s_farPlayerTags is set (sp/FastMenus.cc).
// r12 is free here.

PATCH_B_START(FarTags_807f17b0, 0x0)
    lis r12, s_farPlayerTags@ha
    lbz r12, s_farPlayerTags@l (r12)
    cmpwi r12, 0
    lfs f0, 0x0 (r24) // Original instruction
    beq 1f
    lis r12, s_farTagDistance@ha
    lfs f0, s_farTagDistance@l (r12)
1:
    b FarTags_807f17b0 + 0x4
PATCH_B_END(FarTags_807f17b0, 0x0)

PATCH_B_START(FarTags_807f181c, 0x0)
    lis r12, s_farPlayerTags@ha
    lbz r12, s_farPlayerTags@l (r12)
    cmpwi r12, 0
    lfs f0, 0x0 (r24) // Original instruction
    beq 1f
    lis r12, s_farTagDistance@ha
    lfs f0, s_farTagDistance@l (r12)
1:
    b FarTags_807f181c + 0x4
PATCH_B_END(FarTags_807f181c, 0x0)

// The same distance again (7500, StaticR .data 0x808D3E18), loaded by the tag manager's own
// visibility checks.
PATCH_B_START(FarTags_807f3a24, 0x0)
    lis r12, s_farPlayerTags@ha
    lbz r12, s_farPlayerTags@l (r12)
    cmpwi r12, 0
    lfs f0, 0x0 (r3) // Original instruction
    beq 1f
    lis r12, s_farTagDistance@ha
    lfs f0, s_farTagDistance@l (r12)
1:
    b FarTags_807f3a24 + 0x4
PATCH_B_END(FarTags_807f3a24, 0x0)

PATCH_B_START(FarTags_807f3da4, 0x0)
    lis r12, s_farPlayerTags@ha
    lbz r12, s_farPlayerTags@l (r12)
    cmpwi r12, 0
    lfs f0, 0x0 (r4) // Original instruction
    beq 1f
    lis r12, s_farTagDistance@ha
    lfs f0, s_farTagDistance@l (r12)
1:
    b FarTags_807f3da4 + 0x4
PATCH_B_END(FarTags_807f3da4, 0x0)
