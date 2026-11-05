#include <Common.S>

// TextBox_setMessage(textBox, group, commonGroup, messageId, info): in mogi rooms the "VR"
// messages are swapped for "LR" ones (MogiRemapMessage), and once the message is set the LR
// is coloured by rank (MogiAfterMessage; setting the message resets the colours).
PATCH_B_START(TextBox_setMessage, 0x0)
    stwu r1, -0x20 (r1)
    mflr r0
    stw r0, 0x24 (r1)
    stw r3, 0x8 (r1)
    stw r4, 0xc (r1)
    stw r5, 0x10 (r1)
    stw r7, 0x14 (r1)
    // MogiRemapMessage(textBox, messageId, info)
    mr r4, r6
    mr r5, r7
    bl MogiRemapMessage
    mr r6, r3
    lwz r3, 0x8 (r1)
    lwz r4, 0xc (r1)
    lwz r5, 0x10 (r1)
    lwz r7, 0x14 (r1)
    bl 1f
    // MogiAfterMessage(textBox)
    lwz r3, 0x8 (r1)
    bl MogiAfterMessage
    lwz r0, 0x24 (r1)
    mtlr r0
    addi r1, r1, 0x20
    blr

1:
    stwu r1, -0x120 (r1) // Original instruction
    b TextBox_setMessage + 0x4
PATCH_B_END(TextBox_setMessage, 0x0)
