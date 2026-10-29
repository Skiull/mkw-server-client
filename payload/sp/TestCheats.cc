#include "TestCheats.hh"

extern "C" {
#include <revolution.h>
}

// Test-only Gecko code support, for testing mkw-server's anti-cheat with forum cheat codes
// when the game is started from boot.dol (Dolphin only applies its own cheats to the disc).
// build.py --test_cheats <file> fills in the codes; normal builds have none.
//
// Supported code types (all the test codes use):
//   00aaaaaa nnnn00vv   write the byte vv to 0x80aaaaaa, n+1 bytes in a row
//   02aaaaaa nnnnvvvv   write the halfword vvvv to 0x80aaaaaa, n+1 halfwords in a row
//   04aaaaaa vvvvvvvv   write 32 bits to 0x80aaaaaa
//   C2aaaaaa nnnnnnnn   insert n lines of ASM at 0x80aaaaaa (the last word branches back)
//   C0000000 nnnnnnnn   run n lines of ASM every frame (ends with blr)

namespace SP::TestCheats {

static const u32 s_codes[] = {
#include <TestCheatsCodes.inc>
        0xffffffff,
};

static constexpr u32 MAX_FRAME_CODES = 8;
static const u32 *s_frameCodes[MAX_FRAME_CODES];
static u32 s_frameCodeCount = 0;

// Inserted ASM is copied here so it can branch back.
static u32 s_caves[2048] __attribute__((aligned(32)));
static u32 s_caveUsed = 0;

static u32 branch(const void *from, const void *to) {
    s32 offset = reinterpret_cast<const u8 *>(to) - reinterpret_cast<const u8 *>(from);
    return 0x48000000 | (offset & 0x03fffffc);
}

static void flush(void *p, u32 size) {
    DCFlushRange(p, size);
    ICInvalidateRange(p, size);
}

void Apply() {
    u32 i = 0;
    while (s_codes[i] != 0xffffffff) {
        u32 type = s_codes[i] >> 24 & 0xfe;
        u32 *addr = reinterpret_cast<u32 *>(0x80000000 | (s_codes[i] & 0x01ffffff));
        u32 value = s_codes[i + 1];
        switch (type) {
        case 0x00: {
            u8 *p = reinterpret_cast<u8 *>(addr);
            u32 count = (value >> 16) + 1;
            for (u32 k = 0; k < count; k++) {
                p[k] = value & 0xff;
            }
            flush(p, count);
            i += 2;
            break;
        }
        case 0x02: {
            u16 *p = reinterpret_cast<u16 *>(addr);
            u32 count = (value >> 16) + 1;
            for (u32 k = 0; k < count; k++) {
                p[k] = value & 0xffff;
            }
            flush(p, count * 2);
            i += 2;
            break;
        }
        case 0x04:
            *addr = value;
            flush(addr, 4);
            i += 2;
            break;
        case 0xc2: {
            u32 words = value * 2;
            if (s_caveUsed + words > sizeof(s_caves) / sizeof(u32)) {
                SP_LOG("TestCheats: out of cave space");
                return;
            }
            u32 *cave = &s_caves[s_caveUsed];
            for (u32 k = 0; k < words; k++) {
                cave[k] = s_codes[i + 2 + k];
            }
            cave[words - 1] = branch(&cave[words - 1], addr + 1);
            flush(cave, words * 4);
            *addr = branch(addr, cave);
            flush(addr, 4);
            s_caveUsed += words;
            i += 2 + words;
            break;
        }
        case 0xc0:
            if (s_frameCodeCount < MAX_FRAME_CODES) {
                s_frameCodes[s_frameCodeCount++] = &s_codes[i + 2];
                flush(const_cast<u32 *>(&s_codes[i + 2]), value * 8);
            }
            i += 2 + value * 2;
            break;
        default:
            SP_LOG("TestCheats: unsupported code type %02x, stopping", type);
            return;
        }
    }
    if (i > 0) {
        SP_LOG("TestCheats: applied %u code words", i);
    }
}

void RunFrame() {
    for (u32 k = 0; k < s_frameCodeCount; k++) {
        reinterpret_cast<void (*)()>(const_cast<u32 *>(s_frameCodes[k]))();
    }
}

} // namespace SP::TestCheats
