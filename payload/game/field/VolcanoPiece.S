#include <Common.S>

// Grumble Volcano road pieces (VolcanoPiece) that never fall (TANoFallingRoad): these functions
// work out the piece's state (rest, shake, quake, fall, gone) inline instead of calling
// calcState, so force "rest" (0) there while Field_noFallingRoad is set. Found by
// disassembling the USA StaticR.rel (USA = PAL - 0x12dac here). r12 is reloaded right after.

// Collision checks (virtual, sphere and point variants): state in r25, then
// lwz r12, 0(r26) loads the vtable.
PATCH_B_START(VolcanoPiece_checkCollisionA, 0xa4)
    lis r12, Field_noFallingRoad@ha
    lbz r12, Field_noFallingRoad@l(r12)
    cmpwi r12, 0x0
    beq 1f
    li r25, 0x0
1:
    lwz r12, 0x0(r26) // Original instruction
    b VolcanoPiece_checkCollisionA + 0xa8
PATCH_B_END(VolcanoPiece_checkCollisionA, 0xa4)

PATCH_B_START(VolcanoPiece_checkCollisionB, 0xa4)
    lis r12, Field_noFallingRoad@ha
    lbz r12, Field_noFallingRoad@l(r12)
    cmpwi r12, 0x0
    beq 1f
    li r25, 0x0
1:
    lwz r12, 0x0(r26) // Original instruction
    b VolcanoPiece_checkCollisionB + 0xa8
PATCH_B_END(VolcanoPiece_checkCollisionB, 0xa4)

// The collision scale for a frame: state in r6, then cmpwi r6, 1.
PATCH_B_START(VolcanoPiece_getScale, 0x6c)
    lis r12, Field_noFallingRoad@ha
    lbz r12, Field_noFallingRoad@l(r12)
    cmpwi r12, 0x0
    beq 1f
    li r6, 0x0
1:
    cmpwi r6, 0x1 // Original instruction
    b VolcanoPiece_getScale + 0x70
PATCH_B_END(VolcanoPiece_getScale, 0x6c)
