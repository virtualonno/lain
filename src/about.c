/* The system menu's "About" screen, which scrolls the staff list (BIN.BIN parts 15-17)
   over parts 18 and 19, and the loaders that read a TIM from a BIN.BIN part. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <memory.h>
#include "about.h"
#include "cd.h"
#include "graphics.h"
#include "heap.h"
#include "file_parts.h"
#include "napk.h"
#include "sound.h"

void UploadAboutWindow(int pos, u_long *filler, u_long *first, u_long *second, u_long *third)
{
    RECT rect;
    TIM_IMAGE tim;
    int height;
    int y;

    y = pos;
    if (y == 0) {
        return;
    }
    if (y < 240) {
        setRECT(&rect, 448, 0, 80, 240);
        LoadImage(&rect, filler);
        OpenTIM(first);
        ReadTIM(&tim);
        setRECT(&rect, 448, 240 - y, 80, y);
        LoadImage(&rect, tim.paddr);
    } else if (y <= 512) {
        OpenTIM(first);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, 240);
        LoadImage(&rect, tim.paddr + (y - 240) * 40);
    } else if (y < 752) {
        y -= 240;
        height = 512 - y;
        OpenTIM(first);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, height);
        LoadImage(&rect, tim.paddr + y * 40);
        OpenTIM(second);
        ReadTIM(&tim);
        setRECT(&rect, 448, height, 80, 240 - height);
        LoadImage(&rect, tim.paddr);
    } else if (y < 1024) {
        OpenTIM(second);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, 240);
        LoadImage(&rect, tim.paddr + (y - 752) * 40);
    } else if (y < 1264) {
        y -= 752;
        height = 512 - y;
        OpenTIM(second);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, height);
        LoadImage(&rect, tim.paddr + y * 40);
        OpenTIM(third);
        ReadTIM(&tim);
        setRECT(&rect, 448, height, 80, 240 - height);
        LoadImage(&rect, tim.paddr);
    } else if (y < 1536) {
        OpenTIM(third);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, 240);
        LoadImage(&rect, tim.paddr + (y - 1264) * 40);
    } else {
        setRECT(&rect, 448, 0, 80, 240);
        LoadImage(&rect, filler);
        y = pos - 1264;
        height = 512 - y;
        OpenTIM(third);
        ReadTIM(&tim);
        setRECT(&rect, 448, 0, 80, height);
        LoadImage(&rect, tim.paddr + y * 40);
    }
}

inline void LoadTimPartAt(int part, int x, int y, int clutX, int clutY)
{
    TIM_IMAGE tim;
    RECT rect;
    void *buffer;
    int request;
    u_long *image;

    buffer = AllocHeapBlock(gBinParts[part].size);
    request = RequestFilePartRead(3, part, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[part].size);
    FreeHeapBlock(buffer);
    OpenTIM(image);
    ReadTIM(&tim);
    LoadClut(tim.caddr, clutX, clutY);
    rect.x = x;
    rect.y = y;
    rect.w = tim.prect->w;
    rect.h = tim.prect->h;
    LoadImage(&rect, tim.paddr);
    DrawSync(0);
    FreeHeapBlock(image);
}

inline void LoadTimPart(int part)
{
    TIM_IMAGE tim;
    RECT rect;
    void *buffer;
    int request;
    u_long *image;

    buffer = AllocHeapBlock(gBinParts[part].size);
    request = RequestFilePartRead(3, part, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[part].size);
    FreeHeapBlock(buffer);
    OpenTIM(image);
    ReadTIM(&tim);
    LoadClut(tim.caddr, tim.crect->x, tim.crect->y);
    rect.x = tim.prect->x;
    rect.y = tim.prect->y;
    rect.w = tim.prect->w;
    rect.h = tim.prect->h;
    LoadImage(&rect, tim.paddr);
    DrawSync(0);
    FreeHeapBlock(image);
}

inline u_long *LoadTimPartClut(int part, int clutX, int clutY)
{
    TIM_IMAGE tim;
    RECT rect;
    void *buffer;
    int request;
    u_long *image;

    buffer = AllocHeapBlock(gBinParts[part].size);
    request = RequestFilePartRead(3, part, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[part].size);
    FreeHeapBlock(buffer);
    OpenTIM(image);
    ReadTIM(&tim);
    LoadClut(tim.caddr, clutX, clutY);
    return image;
}

void RunAboutScreen(void)
{
    u_long *strips[3];
    POLY_FT4 panels[4];
    POLY_FT4 tiles[20];
    RECT screen;
    u_long *filler;
    POLY_FT4 *tile;
    int i, frame;
    int u, rightU;

    screen.w = 640;
    screen.x = 0;
    screen.y = 0;
    screen.h = 480;
    ClearImage(&screen, 0, 0, 0);
    FadeOutMusic();
    strips[1] = LoadTimPartClut(16, 320, 242);
    strips[2] = LoadTimPartClut(17, 320, 242);
    strips[0] = LoadTimPartClut(15, 320, 242);
    LoadTimPartAt(19, 320, 0, 320, 240);
    LoadTimPartAt(18, 384, 0, 320, 241);

    SelectMusic(1);
    PlayMusic();
    setPolyFT4(&panels[0]);
    setPolyFT4(&panels[1]);
    setPolyFT4(&panels[2]);
    setPolyFT4(&panels[3]);
    setcode(&panels[1], 0x2e);
    setcode(&panels[2], 0x2e);
    setcode(&panels[3], 0x2e);
    for (frame = 0; frame < 20; frame++) {
        setPolyFT4(&tiles[frame]);
        tile = &tiles[frame];
        tile->tpage = getTPage(0, 0, 320, 0);
        tile->clut = getClut(320, 240);
        setRGB0(tile, 128, 128, 128);
        setUV4(tile, 0, 0, 63, 0, 0, 63, 63, 63);
        tile->x0 = (frame % 5) * 64;
        tile->y0 = (frame / 5) * 64;
        tile->x1 = tile->x0 + 64;
        tile->y1 = tile->y0;
        tile->x2 = tile->x0;
        tile->y2 = tile->y0 + 64;
        tile->x3 = tile->x0 + 64;
        tile->y3 = tile->y0 + 64;
    }
    panels[1].tpage = getTPage(0, 2, 384, 0);
    panels[1].clut = getClut(320, 241);
    setRGB0(&panels[1], 128, 128, 128);
    setXY4(&panels[1], 0, 0, 320, 0, 0, 240, 320, 240);
    setUV4(&panels[1], 0, 0, 7, 0, 0, 239, 7, 239);
    panels[2].tpage = getTPage(0, 0, 448, 0);
    panels[2].clut = getClut(320, 242);
    setRGB0(&panels[2], 128, 128, 128);
    setXY4(&panels[2], 0, 0, 256, 0, 0, 240, 256, 240);
    setUV4(&panels[2], 0, 0, 255, 0, 0, 240, 255, 240);
    panels[3].tpage = getTPage(0, 0, 512, 0);
    panels[3].clut = getClut(320, 242);
    setRGB0(&panels[3], 128, 128, 128);
    setXY4(&panels[3], 256, 0, 320, 0, 256, 240, 320, 240);
    setUV4(&panels[3], 0, 0, 63, 0, 0, 240, 63, 240);

    filler = (u_long *)AllocHeapBlock(160 * 240);
    memset(filler, 0, 160 * 240);
    for (frame = 0; frame < 240; frame++) {
        memset((u_char *)filler + frame * 160, 0x77, 62);
    }
    for (frame = 0; frame < 1776; ) {
        int tileFrame;

        DrawSync(0);
        BeginFrame();
        tileFrame = frame % 4;
        UploadAboutWindow(frame++, filler, strips[0], strips[1], strips[2]);
        i = 0;
        u = tileFrame * 64;
        rightU = u + 63;
        for (; i < 20; i++) {
            tile = &tiles[i];
            tile->u0 = u;
            tile->u1 = rightU;
            tile->u2 = u;
            tile->u3 = rightU;
            GsSortPoly(tile, &gSubOt[gActiveBuffer], 3);
        }
        GsSortPoly(&panels[1], &gSubOt[gActiveBuffer], 0);
        GsSortPoly(&panels[3], &gSubOt[gActiveBuffer], 1);
        GsSortPoly(&panels[2], &gSubOt[gActiveBuffer], 1);
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
    }
    DrawSync(0);
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    FadeOutMusic();
    screen.w = 640;
    screen.x = 0;
    screen.y = 0;
    screen.h = 480;
    ClearImage(&screen, 0, 0, 0);
    DrawSync(0);
    SelectMusic(0);
    PlayMusic();
    FreeHeapBlock(strips[0]);
    FreeHeapBlock(strips[1]);
    FreeHeapBlock(strips[2]);
    FreeHeapBlock(filler);
}
