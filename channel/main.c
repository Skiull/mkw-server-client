// MKW Self-host channel: a small menu that boots the embedded boot.dol (the
// mkw-server-client stub) the same way the Homebrew Channel launches it.
//
// Built and packed into the .wad by tools/make_wad.py.

#include <gccore.h>
#include <ogc/machine/processor.h>
#include <stdio.h>
#include <string.h>
#include <wiiuse/wpad.h>

extern const u8 boot_dol[];
extern const u8 boot_dol_end[];
extern void relocator(const void *dol);
extern const u8 relocator_end[];

// Where the DOL is staged before launch and where the relocator runs from. Both
// are in MEM2, away from where the DOL loads and below IOS's area at the top.
#define STAGED_DOL ((void *)0x92000000)
#define RELOCATOR_ADDR ((void *)0x92800000)

// What the boot stub (tools/channel_bootstub.S) saved from the System Menu at boot
#define VI_SAVE ((const vu16 *)0x80003680) // VI registers
#define OS_SAVE ((const vu32 *)0x80003700) // OS globals 0x80000000-0x800000FF

static GXRModeObj *rmode;
static void *xfb;

enum { ITEM_LAUNCH, ITEM_COUNT };
static const char *items[ITEM_COUNT] = {"Launch Game"};

static void initVideo(void) {
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
            rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }
}

static int consoleCols(void) {
    int cols, rows;
    CON_GetMetrics(&cols, &rows);
    return cols;
}

static void printCentered(int row, const char *text) {
    int col = (consoleCols() - (int)strlen(text)) / 2;
    printf("\x1b[%d;%dH%s", row, col < 0 ? 0 : col, text);
}

static void drawMenu(int selected) {
    printf("\x1b[40m\x1b[2J");
    printf("\x1b[37;1m");
    printCentered(3, "MARIO KART WII");
    printf("\x1b[36;1m");
    printCentered(4, "Self-host Server");
    printf("\x1b[37m");
    printCentered(5, "--------------------------------");

    for (int i = 0; i < ITEM_COUNT; i++) {
        char button[48];
        snprintf(button, sizeof(button), "   %-20s   ", items[i]);
        if (i == selected) {
            printf("\x1b[44m\x1b[37;1m");
        } else {
            printf("\x1b[40m\x1b[37m");
        }
        printCentered(9 + i * 2, button);
    }

    printf("\x1b[40m\x1b[33m");
    printCentered(20, "A: Select    HOME: Wii Menu");
#ifdef DIAG
    {
        char line[96];
        snprintf(line, sizeof(line), "saved CC=%08x 28=%08x F8=%08x  VI dcr=%04x", OS_SAVE[0xcc / 4],
                OS_SAVE[0x28 / 4], OS_SAVE[0xf8 / 4], VI_SAVE[1]);
        printCentered(23, line);
        snprintf(line, sizeof(line), "now   CC=%08x 28=%08x F8=%08x  libogc mode=%u",
                *(vu32 *)0x800000cc, *(vu32 *)0x80000028, *(vu32 *)0x800000f8, rmode->viTVMode);
        printCentered(24, line);
    }
#endif
    printf("\x1b[37m");
}

static void showStatus(const char *text) {
    printf("\x1b[40m\x1b[37;1m");
    printCentered(16, text);
}

// Point every exception vector at a bare `rfi`, so nothing jumps back into
// libogc's handlers once the DOL overwrites the menu. Same as the old libogc
// __exception_closeall(), which libogc 3 no longer has.
static void closeExceptions(void) {
    static const u32 vectors[] = {0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800,
            0x900, 0xc00, 0xd00, 0xf00, 0x1300, 0x1400, 0x1700};
    for (u32 i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
        u32 *vector = (u32 *)(0x80000000 | vectors[i]);
        *vector = 0x4c000064; // rfi
        DCFlushRange(vector, 0x20);
        ICInvalidateRange(vector, 0x20);
    }
}

#ifdef DIAG
// Diagnostic build (make_wad.py --diag): show a solid colour at each hand-off
// stage on a framebuffer at 0x81700000 (clear of the stub's at 0x81600000).
#define DIAG_XFB ((u32 *)0xC1700000)
static void diagFill(u32 colour) {
    for (u32 i = 0; i < 320 * 480; i++) {
        DIAG_XFB[i] = colour;
    }
    u32 start, now;
    asm volatile("mftbl %0" : "=r"(start));
    do {
        asm volatile("mftbl %0" : "=r"(now));
    } while (now - start < 91000000); // ~1.5 s
}
#endif

// The boot stub (tools/channel_bootstub.S) saves the System Menu's VI registers
// here at boot. The payload stub's VI::Init (common/VI.cc) only rewrites a few
// registers and relies on the rest, and on a real Wii it only works on top of the
// System Menu's setup, not libogc's. So put the System Menu's setup back.
static void restoreSystemMenuVideo(void) {
    volatile u16 *vi = (volatile u16 *)0xCC002000;
    if (!(VI_SAVE[1] & 1)) {
        return; // VI wasn't running at boot (Dolphin): keep libogc's setup
    }
    for (u32 i = 0; i < 64; i++) {
        // skip DCR (written last) and the read-only beam position registers
        if (i == 1 || i == 0x2c / 2 || i == 0x2e / 2) {
            continue;
        }
        vi[i] = VI_SAVE[i];
    }
    vi[1] = VI_SAVE[1];
}

// The System Menu's OS globals (0x80000000-0x800000FF), saved by the boot stub.
// libogc rewrites these (TV mode at 0xCC among them); a disc game expects the
// System Menu's values, which is what it gets when booted from the disc channel.
static void restoreSystemMenuGlobals(void) {
    if (OS_SAVE[0x28 / 4] == 0) {
        return; // nothing saved (Dolphin starts with empty memory)
    }
    // 0x00-0x1F is the disc header, which the loader fills in itself
    for (u32 i = 0x20 / 4; i < 0x100 / 4; i++) {
        ((vu32 *)0x80000000)[i] = OS_SAVE[i];
    }
    DCFlushRange((void *)0x80000000, 0x100);
}

static void launchGame(void) {
    u32 size = boot_dol_end - boot_dol;
    showStatus("Starting Mario Kart Wii...");

    // Stage the DOL in MEM2 so loading it can't overwrite its own source
    memcpy(STAGED_DOL, boot_dol, size);
    DCFlushRange(STAGED_DOL, size);

    // Same hand-off as the Homebrew Channel (hbc/channel/channelapp: gfx_deinit,
    // main() and loader_exec): black out the video, relaunch IOS58, shut libogc
    // down, close the exception vectors, poke the clock speeds, then jump.
    WPAD_Shutdown();
#ifdef DIAG
    VIDEO_SetNextFramebuffer(DIAG_XFB);
    VIDEO_Flush();
    VIDEO_WaitVSync();
#else
    VIDEO_SetBlack(true);
#endif
    VIDEO_Flush();
    for (int i = 0; i < 4; i++) {
        VIDEO_WaitVSync();
    }
    __IOS_LaunchNewIOS(58); // if this fails we keep the channel's own IOS58
    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

    u32 level;
    _CPU_ISR_Disable(level);
    (void)level;
    closeExceptions();

    // "these pokes make ninty SDK dols work" (HBC)
    *(vu32 *)0x800000F8 = 0x0E7BE2C0; // bus clock
    *(vu32 *)0x800000FC = 0x2B73A840; // CPU clock
#ifdef DIAG
    diagFill(0x515a51f0); // red: menu shut down, about to jump
#endif
    restoreSystemMenuVideo();
    restoreSystemMenuGlobals();

    // Copy the relocator last, once nothing else will touch memory
    u32 relocatorSize = relocator_end - (const u8 *)relocator;
    memcpy(RELOCATOR_ADDR, relocator, relocatorSize);
    DCFlushRange(RELOCATOR_ADDR, relocatorSize);
    ICInvalidateRange(RELOCATOR_ADDR, relocatorSize);
    ((void (*)(const void *))RELOCATOR_ADDR)(STAGED_DOL);
}

int main(void) {
    initVideo();
    WPAD_Init();
    PAD_Init();

    int selected = 0;
    drawMenu(selected);

    while (true) {
        WPAD_ScanPads();
        PAD_ScanPads();
        u32 wii = 0;
        for (int i = 0; i < WPAD_MAX_WIIMOTES; i++) {
            wii |= WPAD_ButtonsDown(i);
        }
        u32 gc = 0;
        for (int i = 0; i < PAD_CHANMAX; i++) {
            gc |= PAD_ButtonsDown(i);
        }

        if (wii & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) {
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }
        int moved = selected;
        if ((wii & (WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP)) || (gc & PAD_BUTTON_UP)) {
            selected = (selected + ITEM_COUNT - 1) % ITEM_COUNT;
        }
        if ((wii & (WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN)) || (gc & PAD_BUTTON_DOWN)) {
            selected = (selected + 1) % ITEM_COUNT;
        }
        if (moved != selected) {
            drawMenu(selected);
        }
        if ((wii & (WPAD_BUTTON_A | WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_A)) ||
                (gc & (PAD_BUTTON_A | PAD_BUTTON_START))) {
            if (selected == ITEM_LAUNCH) {
                launchGame();
            }
        }
        VIDEO_WaitVSync();
    }
}
