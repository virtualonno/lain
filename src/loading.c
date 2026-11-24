/* BIN.BIN TIM loader, 29-frame loading animation ("make me sad" at boot, "life instinct
 * function OS" in-game), and the P2-01..P2-06 assembly screen. */
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "about.h"
#include "loading.h"
#include "cd.h"
#include "graphics.h"
#include "grid_menu.h"
#include "heap.h"
#include "file_parts.h"
#include "kana_entry.h"
#include "napk.h"
#include "pad.h"
#include "sound.h"
#include "text.h"

short gUnusedShort = 0; /* 2 bytes; nothing reads or writes it */
enum TimLoadStep {
    TIM_LOAD_START,
    TIM_LOAD_DRAW_FIRST,
    TIM_LOAD_REQUEST_START_CHOICES,
    TIM_LOAD_UPLOAD_START_CHOICES,
    TIM_LOAD_REQUEST_KANA_CHART_MID,
    TIM_LOAD_UPLOAD_KANA_CHART_MID,
    TIM_LOAD_REQUEST_KANA_CHART_END,
    TIM_LOAD_UPLOAD_KANA_CHART_END,
    TIM_LOAD_REQUEST_KANA_CHART_TOP,
    TIM_LOAD_UPLOAD_KANA_CHART_TOP,
    TIM_LOAD_REQUEST_KANA_UI,
    TIM_LOAD_UPLOAD_KANA_UI,
    TIM_LOAD_REQUEST_KANA_GLYPHS,
    TIM_LOAD_UPLOAD_KANA_GLYPHS,
    TIM_LOAD_REQUEST_MESSAGE_WINDOW,
    TIM_LOAD_UPLOAD_MESSAGE_WINDOW,
    TIM_LOAD_REQUEST_JUMP_TO,
    TIM_LOAD_UPLOAD_JUMP_TO,
    TIM_LOAD_REQUEST_DISC_CHANGE,
    TIM_LOAD_UPLOAD_DISC_CHANGE,
    TIM_LOAD_REQUEST_CONTINUE_PROMPT,
    TIM_LOAD_UPLOAD_CONTINUE_PROMPT,
    TIM_LOAD_REQUEST_START_STRIP,
    TIM_LOAD_UPLOAD_START_STRIP,
    TIM_LOAD_REQUEST_START_OPENING,
    TIM_LOAD_UPLOAD_START_OPENING
};
/* Each BIN.BIN part takes a request step and an upload step; TIM_LOAD_START skips the loading frame. */
short gTimLoadStep = TIM_LOAD_START;
short gLoadingFrame = 0;
static int sUnusedWord = 0; /* 4 bytes; nothing reads or writes it */
short gTimLoadPart;
short gTimLoadRequest;
void *gTimLoadBuffer;

GsSPRITE gP2Sprites[16] = {
    {
        0x00000000, 24, 32, 0x00000078, 0x00000020, 0x00000016, 0x00000000,
        0x00000000, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 152, 168, 0x00000090, 0x00000030, 0x00000016, 0x00000068,
        0x00000010, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 184, 136, 0x00000070, 0x00000030, 0x00000016, 0x00000068,
        0x00000060, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 200, 88, 0x00000060, 0x00000020, 0x00000016, 0x00000078,
        0x00000040, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 24, 120, 0x00000060, 0x00000028, 0x00000016, 0x00000000,
        0x00000020, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 24, 192, 0x00000058, 0x00000018, 0x00000016, 0x00000000,
        0x00000050, 384, 400, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 174, 98, 0x00000030, 0x00000038, 0x00000085, 0x00000000,
        0x00000000, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 118, 50, 0x00000050, 0x00000050, 0x00000085, 0x00000030,
        0x00000000, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 126, 122, 0x00000040, 0x00000038, 0x00000085, 0x00000080,
        0x00000000, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 166, 170, 0x00000038, 0x00000020, 0x00000085, 0x000000c0,
        0x00000000, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 102, 170, 0x00000030, 0x00000020, 0x00000085, 0x00000000,
        0x00000050, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 94, 122, 0x00000038, 0x00000040, 0x00000085, 0x00000030,
        0x00000050, 320, 144, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x40000000, 96, 48, 0x00000080, 0x000000a0, 0x00000035, 0x00000000,
        0x00000000, 320, 416, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 200, 0, 0x00000078, 0x00000068, 0x00000087, 0x00000000,
        0x00000000, 448, 112, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x01000000, 0, 128, 0x00000080, 0x00000070, 0x00000087, 0x00000078,
        0x00000000, 448, 112, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
    {
        0x00000000, 24, 212, 0x00000058, 0x00000010, 0x00000018, 0x00000000,
        0x00000000, 0, 481, 0x00000080, 0x00000080, 0x00000080, 0,
        0, 0, 0, 0,
    },
};

void LoadP2Screen(void)
{
    int pieceClutY;
    int clutY;
    int width;
    unsigned int centerWidth;

    pieceClutY = 0x90;
    LoadTimPartAt(0x21, 0x140, 0, 0x140, pieceClutY);
    LoadTimPartAt(0x25, 0x158, 0, 0x140, pieceClutY);
    LoadTimPartAt(0x20, 0x180, 0, 0x140, pieceClutY);
    LoadTimPartAt(0x23, 0x1a0, 0, 0x140, pieceClutY);
    LoadTimPartAt(0x24, 0x140, 0x50, 0x140, pieceClutY);
    LoadTimPartAt(0x22, 0x158, 0x50, 0x140, pieceClutY);
    clutY = 0x70;
    LoadTimPartAt(0x26, 0x1c0, 0, 0x1c0, clutY);
    LoadTimPartAt(0x27, 0x1fc, 0, 0x1c0, clutY);
    LoadTimPartAt(0x29, 0x140, 0x100, 0x140, 0x1a0);
    LoadTimPartAt(0x28, 0x180, 0x100, 0x180, 0x190);
    width = UploadTextToVram((u_char *)"press ANY button", 0x200, 0x100, TEXT_UPLOAD_SMALL, 1);
    centerWidth = gP2Sprites[5].w;
    gP2Sprites[15].w = width;
    gP2Sprites[15].x = 0x88 - centerWidth / 2;
}

int RunP2Screen(void)
{
    RECT rect;
    int i;
    int frames;
    int blink;

    setRECT(&rect, 0, 0, 320, 480);
    ClearImage(&rect, 0, 0, 0);
    frames = 0;
    LoadP2Screen();
    blink = 0;
    if (gP2NodesOpened == 63) {
        PlaySound(30);
    }
    for (;;) {
        DrawSync(0);
        BeginFrame();
        ReadPadEvent();
        for (i = 0; i < 15; i++) {
            switch (i) {
            case 6:
                if (gP2NodesOpened & 8) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            case 7:
                if (gP2NodesOpened & 1) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            case 8:
                if (gP2NodesOpened & 2) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            case 9:
                if (gP2NodesOpened & 4) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            case 10:
                if (gP2NodesOpened & 0x10) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            case 11:
                if (gP2NodesOpened & 0x20) {
                    GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                }
                break;
            default:
                GsSortFastSprite(&gP2Sprites[i], &gSubOt[gActiveBuffer], 1);
                break;
            }
        }
        if (frames >= 30) {
            if (gPadEvent != PAD_EVENT_NONE) {
                break;
            }
            if (blink < 30) {
                GsSortFastSprite(&gP2Sprites[15], &gSubOt[gActiveBuffer], 0);
            }
            blink++;
            if (blink == 60) {
                blink = 0;
            }
        }
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
        if (frames < 30) {
            frames++;
        }
    }
    setRECT(&rect, 0, 0, 320, 480);
    return ClearImage(&rect, 0, 0, 0);
}

int StepTimLoadSequence(int discardPart14)
{
    TIM_IMAGE tim;
    RECT rect;

    switch (gTimLoadStep) {
    case TIM_LOAD_REQUEST_START_CHOICES:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[13].size);
        gTimLoadPart = 13;
        gTimLoadRequest = RequestFilePartRead(3, 13, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_START_CHOICES: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 489);
        rect.x = 320;
        rect.y = 409;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_KANA_CHART_MID:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[6].size);
        gTimLoadPart = 6;
        gTimLoadRequest = RequestFilePartRead(3, 6, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_KANA_CHART_MID: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 241);
        rect.x = 384;
        rect.y = 0;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_KANA_CHART_END:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[7].size);
        gTimLoadPart = 7;
        gTimLoadRequest = RequestFilePartRead(3, 7, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_KANA_CHART_END: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 241);
        rect.x = 344;
        rect.y = 0;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_KANA_CHART_TOP:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[5].size);
        gTimLoadPart = 5;
        gTimLoadRequest = RequestFilePartRead(3, 5, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_KANA_CHART_TOP: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 241);
        rect.x = 448;
        rect.y = 0;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_KANA_UI:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[4].size);
        gTimLoadPart = 4;
        gTimLoadRequest = RequestFilePartRead(3, 4, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_KANA_UI: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 242);
        rect.x = 384;
        rect.y = 256;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_KANA_GLYPHS:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[8].size);
        gTimLoadPart = 8;
        gTimLoadRequest = RequestFilePartRead(3, 8, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_KANA_GLYPHS: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 243);
        rect.x = 384;
        rect.y = 321;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_MESSAGE_WINDOW:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[22].size);
        gTimLoadPart = 22;
        gTimLoadRequest = RequestFilePartRead(3, 22, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_MESSAGE_WINDOW: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 0, 485);
        rect.x = 896;
        rect.y = 256;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_JUMP_TO:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[23].size);
        gTimLoadPart = 23;
        gTimLoadRequest = RequestFilePartRead(3, 23, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_JUMP_TO: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 0, 486);
        rect.x = 832;
        rect.y = 256;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_DISC_CHANGE:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[24].size);
        gTimLoadPart = 24;
        gTimLoadRequest = RequestFilePartRead(3, 24, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_DISC_CHANGE: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 0, 487);
        rect.x = 768;
        rect.y = 256;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_CONTINUE_PROMPT:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[31].size);
        gTimLoadPart = 31;
        gTimLoadRequest = RequestFilePartRead(3, 31, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_CONTINUE_PROMPT: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 0, 488);
        rect.x = 768;
        rect.y = 408;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep++;
        break;
    }
    case TIM_LOAD_REQUEST_START_STRIP:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[14].size);
        gTimLoadPart = 14;
        gTimLoadRequest = RequestFilePartRead(3, 14, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_START_STRIP: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        if (!discardPart14) {
            texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
            FreeHeapBlock(gTimLoadBuffer);
            OpenTIM(texture);
            ReadTIM(&tim);
            LoadClut(tim.caddr, 320, 244);
            DrawSync(0);
            gStripTim = texture;
        } else {
            FreeHeapBlock(gTimLoadBuffer);
        }
        /* fall through */
    }
    case TIM_LOAD_START:
    case TIM_LOAD_DRAW_FIRST:
        gTimLoadStep++;
        break;
    case TIM_LOAD_REQUEST_START_OPENING:
        gTimLoadBuffer = AllocHeapBlock(gBinParts[3].size);
        gTimLoadPart = 3;
        gTimLoadRequest = RequestFilePartRead(3, 3, gBinParts, gTimLoadBuffer);
        gTimLoadStep++;
        break;
    case TIM_LOAD_UPLOAD_START_OPENING: {
        u_long *texture;

        if (PollCdRequest(gTimLoadRequest) == 0) {
            break;
        }
        texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
        FreeHeapBlock(gTimLoadBuffer);
        OpenTIM(texture);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 408);
        rect.x = 320;
        rect.y = 256;
        rect.w = tim.prect->w;
        rect.h = tim.prect->h;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(texture);
        gTimLoadStep = TIM_LOAD_START;
        gLoadingFrame = 0;
        return 1;
    }
    }
    if (gTimLoadStep >= TIM_LOAD_REQUEST_START_CHOICES) {
        DrawLoadingFrame(0);
    }
    return 0;
}

void LoadLoadingImage(void)
{
    TIM_IMAGE tim;
    RECT rect;
    u_long *texture;

    gTimLoadBuffer = AllocHeapBlock(gBinParts[42].size);
    gTimLoadPart = 42;
    gTimLoadRequest = RequestFilePartRead(3, 42, gBinParts, gTimLoadBuffer);
    while (PollCdRequest(gTimLoadRequest) == 0) {
    }
    texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
    FreeHeapBlock(gTimLoadBuffer);
    OpenTIM(texture);
    ReadTIM(&tim);
    LoadClut(tim.caddr, 640, 376);
    rect.x = 640;
    rect.y = 256;
    rect.w = tim.prect->w;
    rect.h = tim.prect->h;
    LoadImage(&rect, tim.paddr);
    DrawSync(0);
    FreeHeapBlock(texture);
    gLoadingFrame = 0;
}

void LoadAltLoadingImage(void)
{
    TIM_IMAGE tim;
    RECT rect;
    u_long *texture;

    gTimLoadBuffer = AllocHeapBlock(gBinParts[43].size);
    gTimLoadPart = 43;
    gTimLoadRequest = RequestFilePartRead(3, 43, gBinParts, gTimLoadBuffer);
    while (PollCdRequest(gTimLoadRequest) == 0) {
    }
    texture = (u_long *)UnpackNapk(gTimLoadBuffer, gBinParts[gTimLoadPart].size);
    FreeHeapBlock(gTimLoadBuffer);
    OpenTIM(texture);
    ReadTIM(&tim);
    rect.x = 640;
    rect.y = 256;
    rect.w = tim.prect->w;
    rect.h = tim.prect->h;
    LoadImage(&rect, tim.paddr);
    DrawSync(0);
    FreeHeapBlock(texture);
    gLoadingFrame = 0;
}

GsSPRITE gLoadingSprites[2] = {
    { 0x00, 148, 84, 0x18, 0x28, 0x1a, 0x00, 0x00, 640, 376, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 144, 128, 0x20, 0x28, 0x1a, 0xd8, 0x50, 640, 376, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
};

int DrawLoadingFrame(int resetFrame)
{
    short column, row;
    int inRange;

    if (resetFrame) {
        gLoadingFrame = 0;
    }
    DrawSync(0);
    BeginFrame();
    column = gLoadingFrame % 10;
    row = gLoadingFrame / 10;
    gLoadingSprites[0].u = column * 24;
    gLoadingSprites[0].v = row * 40;
    GsSortFastSprite(&gLoadingSprites[0], &gSubOt[gActiveBuffer], 0);
    GsSortFastSprite(&gLoadingSprites[1], &gSubOt[gActiveBuffer], 0);
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
    GsDrawOt(&gSubOt[gActiveBuffer]);
    inRange = ++gLoadingFrame < 29;
    if (!inRange) {
        gLoadingFrame = 0;
    }
    return inRange;
}

/* Chroma and luma quantizers: set by clip.c and media_player.c, read by DECODE.s. */
short gChromaQuant;
short gLumaQuant;
