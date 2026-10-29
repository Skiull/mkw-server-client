#include <Common.S>

// The error page loads the error into r3, then calls ExplainDWCError for the message id.
// Call WiiLink_ExplainError instead, with the page's MessageInfo (r1 + 0x10) so it can show
// custom text (e.g. "Invalid game version, please update.").
PATCH_BL_START(ErrorPage_explainDWCErrorCall, 0x0)
    addi r4, r1, 0x10
    b WiiLink_ExplainError
PATCH_BL_END(ErrorPage_explainDWCErrorCall, 0x0)
