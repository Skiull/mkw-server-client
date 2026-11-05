#include <Common.S>

// TextBox_setMessage(textBox, group, commonGroup, messageId, info): let MogiRemapMessage
// change the message id first (mogi texts on the friends page).
PATCH_B_START(TextBox_setMessage, 0x0)
    stwu r1, -0x20 (r1)
    mflr r0
    stw r0, 0x24 (r1)
    stw r3, 0x8 (r1)
    stw r4, 0xc (r1)
    stw r5, 0x10 (r1)
    stw r7, 0x14 (r1)
    mr r3, r6
    bl MogiRemapMessage
    mr r6, r3
    lwz r3, 0x8 (r1)
    lwz r4, 0xc (r1)
    lwz r5, 0x10 (r1)
    lwz r7, 0x14 (r1)
    lwz r0, 0x24 (r1)
    mtlr r0
    addi r1, r1, 0x20

    stwu r1, -0x120 (r1) // Original instruction
    b TextBox_setMessage + 0x4
PATCH_B_END(TextBox_setMessage, 0x0)
