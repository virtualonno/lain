/* Top-level boot and game loop: hardware and subsystem init, start screen, disc check, and the
 * grid loop (enum GridMenuResult). Defines HandleVSync and three unreferenced boot/reset helpers. */
#include <sys/types.h>
#include <libapi.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>

#include "loading.h"
#include "cd.h"
#include "disc_change.h"
#include "graphics.h"
#include "grid_menu.h"
#include "grid_nodes.h"
#include "grid_scene.h"
#include "heap.h"
#include "host_log.h"
#include "kana_entry.h"
#include "lapks_player.h"
#include "main.h"
#include "media_player.h"
#include "memcard.h"
#include "clip.h"
#include "pad.h"
#include "sound.h"
#include "system_menu.h"
#include "text.h"

int main(void)
{
    int siteChange;

    SetMem(2);
    ResetCallback();
    OpenHostLog("d:\\usr\\tmp\\psx.log");
    InitGraphics();
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));
    InitCdRequests();
    InitClipPlaybackSlots();
    InitSound(0);
    InitMemCard();
    InitPads();
    InitText(68, 49);
    LoadMenuTileImage();
    InitGridScene();
    InitGridState();
    BuildNodeGrid();
    LoadGridEntryClip();
    VSyncCallback(HandleVSync);
    siteChange = 0;
    for (;;) {
        HostLogPrintf("1left memory %d\n", GetLeftMemory());
        RunStartScreen();
        HostLogPrintf("2left memory %d\n", GetLeftMemory());
        for (;;) {
            while (PollGridEntryClip() == 0) {
            }
            LoadAltLoadingImage();
            EnsureSiteDisc(siteChange);
            switch (RunGridMenu()) {
            case GRID_MENU_EXIT_LOAD:
                siteChange = 0;
                RestoreGridCursor();
                FreeIdleClips();
                LoadGridEntryClip();
                continue;
            case GRID_MENU_EXIT_SITE_CHANGE:
                siteChange = 1;
                RestoreGridCursor();
                FreeIdleClips();
                LoadGridEntryClip();
                continue;
            case GRID_MENU_EXIT_CONTINUE:
                siteChange = 0;
                RestoreGridCursor();
                SsSetMVol(0, 0);
                LoadLoadingImage();
                while (StepTimLoadSequence(1) == 0) {
                }
                SsSetMVol(127, 127);
                InitText(68, 49);
                FreeIdleClips();
                LoadGridEntryClip();
                continue;
            }
            break;
        }
        siteChange = 0;
        LoadLoadingImage();
        InitGridState();
        ResetNodeStates();
        BuildNodeGrid();
        InitSound(1);
        LoadGridEntryClip();
    }
}

void InitGraphicsAndFont(void)
{
    InitGraphics();
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));
}

void HandleVSync(void)
{
    CountVSync();
    PollXaPlayback();
    SoundVSync();
}

void InitProgram(void)
{
    ResetCallback();
    OpenHostLog("d:\\usr\\tmp\\psx.log");
    InitGraphics();
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));
    InitCdRequests();
    InitClipPlaybackSlots();
    InitSound(0);
    InitMemCard();
    InitPads();
    InitText(68, 49);
    LoadMenuTileImage();
    InitGridScene();
    InitGridState();
    BuildNodeGrid();
    LoadGridEntryClip();
    VSyncCallback(HandleVSync);
}

int ResetForStartScreen(void)
{
    LoadLoadingImage();
    InitGridState();
    ResetNodeStates();
    BuildNodeGrid();
    InitSound(1);
    return LoadGridEntryClip();
}
