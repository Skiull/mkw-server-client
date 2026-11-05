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
// are in MEM2, away from where the DOL loads and below IOS's area (as WiiFlow does).
#define STAGED_DOL ((void *)0x92000000)
#define RELOCATOR_ADDR ((void *)0x93000000)

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

static void launchGame(void) {
    u32 size = boot_dol_end - boot_dol;
    showStatus("Starting Mario Kart Wii...");

    // Stage the DOL in MEM2 so loading it can't overwrite its own source
    memcpy(STAGED_DOL, boot_dol, size);
    DCFlushRange(STAGED_DOL, size);

    // Hand over a clean IOS58, like the Homebrew Channel does
    WPAD_Shutdown();
    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    IOS_ReloadIOS(58); // if this fails we keep the channel's own IOS58
    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

    u32 level;
    _CPU_ISR_Disable(level);
    (void)level;
    closeExceptions();

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
