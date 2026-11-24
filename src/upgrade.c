/* The screen an SSkn node (kind 7) opens to offer an upgrade: up chooses accept, down decline,
 * circle confirms, and accepting runs a twenty-step progress bar before the fade-out. Returns 1
 * when accepted; the grid menu then raises the application version. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include "upgrade.h"
#include "cd.h"
#include "sprite_quad.h"
#include "graphics.h"
#include "heap.h"
#include "file_parts.h"
#include "kana_entry.h"
#include "napk.h"
#include "pad.h"
#include "system_menu.h"


SpriteDesc gUpgradeSprites[21] = {
    { 54, 44, 96, 80, 5, 0, 112, 400, 193 },
    { 228, 173, 32, 16, 5, 152, 0, 320, 193 },
    { 228, 197, 72, 16, 5, 184, 0, 336, 193 },
    { 34, 188, 88, 24, 5, 152, 16, 336, 193 },
    { 8, 20, 128, 16, 5, 0, 72, 320, 193 },
    { 246, 20, 64, 24, 5, 152, 40, 352, 193 },
    { 158, 76, 152, 40, 5, 0, 0, 320, 193 },
    { 158, 124, 152, 32, 5, 0, 40, 320, 193 },
    { 0, 194, 200, 8, 5, 0, 88, 336, 193 },
    { 215, 182, 112, 16, 5, 128, 72, 336, 193 },
    { 215, 205, 112, 16, 5, 128, 72, 320, 193 },
    { 75, 36, 152, 192, 6, 0, 0, 320, 194 },
    { 138, 190, 176, 16, 5, 0, 96, 384, 193 },
    { 0, 0, 8, 8, 5, 152, 64, 368, 193 },
    { 0, 0, 8, 8, 5, 160, 64, 368, 193 },
    { 0, 0, 8, 8, 5, 168, 64, 368, 193 },
    { 0, 0, 8, 8, 5, 176, 64, 368, 193 },
    { 0, 0, 8, 8, 5, 184, 64, 368, 193 },
    { 0, 0, 8, 8, 5, 192, 64, 368, 193 },
    { 54, 44, 96, 80, 5, 0, 112, 400, 193 },
    { 30, 68, 192, 160, 71, 0, 0, 320, 195 },
};

enum UpgradePhase {
    UPGRADE_PHASE_FADE_IN,
    UPGRADE_PHASE_CHOOSE,
    UPGRADE_PHASE_PROGRESS,
    UPGRADE_PHASE_FADE_OUT,
    UPGRADE_PHASE_DONE
};

enum UpgradeChoice {
    UPGRADE_CHOICE_ACCEPT,
    UPGRADE_CHOICE_DECLINE
};

int RunUpgradeScreen(void)
{
    TIM_IMAGE tim;
    RECT rect;
    GsSPRITE sprites[19];
    SpriteQuad quads[2];
    void *packed;
    void *image;
    int i;
    int j;
    int phase;
    int frame;
    int progress;
    int angle;
    int cycle;
    int selection;
    int result;
    u_short clutX;
    u_short clutY;

    rect.w = 320;
    rect.x = 0;
    rect.y = 0;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    packed = AllocHeapBlock(gBinParts[10].size);
    if (packed) {
        frame = RequestFilePartRead(3, 10, gBinParts, packed);
        while (!PollCdRequest(frame)) {
        }
        image = UnpackNapk(packed, gBinParts[10].size);
        FreeHeapBlock(packed);
        OpenTIM(image);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 193);
        rect.x = 320;
        rect.w = 64;
        rect.y = 0;
        rect.h = 192;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(image);

        packed = AllocHeapBlock(gBinParts[11].size);
        frame = RequestFilePartRead(3, 11, gBinParts, packed);
        while (!PollCdRequest(frame)) {
        }
        image = UnpackNapk(packed, gBinParts[11].size);
        FreeHeapBlock(packed);
        OpenTIM(image);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 194);
        rect.x = 384;
        rect.w = 38;
        rect.y = 0;
        rect.h = 192;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(image);

        packed = AllocHeapBlock(gBinParts[12].size);
        frame = RequestFilePartRead(3, 12, gBinParts, packed);
        while (!PollCdRequest(frame)) {
        }
        image = UnpackNapk(packed, gBinParts[12].size);
        FreeHeapBlock(packed);
        OpenTIM(image);
        ReadTIM(&tim);
        LoadClut(tim.caddr, 320, 195);
        rect.x = 448;
        rect.w = 48;
        rect.y = 0;
        rect.h = 160;
        LoadImage(&rect, tim.paddr);
        DrawSync(0);
        FreeHeapBlock(image);
    }
    for (i = 0; i < 19; i++) {
        sprites[i].attribute = 0;
        sprites[i].x = gUpgradeSprites[i].x;
        sprites[i].y = gUpgradeSprites[i].y;
        sprites[i].w = gUpgradeSprites[i].w;
        sprites[i].h = gUpgradeSprites[i].h;
        sprites[i].tpage = gUpgradeSprites[i].tpage;
        sprites[i].u = gUpgradeSprites[i].u;
        sprites[i].v = gUpgradeSprites[i].v;
        sprites[i].cx = gUpgradeSprites[i].cx;
        sprites[i].cy = gUpgradeSprites[i].cy;
        sprites[i].r = 0;
        sprites[i].g = 0;
        sprites[i].b = 0;
        sprites[i].scalex = 4096;
        sprites[i].scaley = 4096;
        sprites[i].rotate = 0;
    }
    sprites[6].attribute = GsALON;
    sprites[7].attribute = GsALON;
    for (i = 0; i < 2; i++) {
        SpriteQuad *entry = &quads[i];
        entry->attribute = 0;
        entry->x = gUpgradeSprites[i + 19].x;
        entry->y = gUpgradeSprites[i + 19].y;
        entry->w = gUpgradeSprites[i + 19].w;
        entry->h = gUpgradeSprites[i + 19].h;
        entry->tpage = gUpgradeSprites[i + 19].tpage;
        entry->u = gUpgradeSprites[i + 19].u;
        entry->v = gUpgradeSprites[i + 19].v;
        clutY = gUpgradeSprites[i + 19].cy;
        clutX = gUpgradeSprites[i + 19].cx;
        entry->r = 0;
        entry->g = 0;
        entry->b = 0;
        entry->scale[0] = 4096;
        entry->scale[1] = 4096;
        entry->rotateX = 0;
        entry->rotateY = 0;
        entry->rotateZ = 0;
        entry->clut = getClut(clutX, clutY);
    }

    quads[1].attribute = 0x40000000;
    angle = 0;
    phase = UPGRADE_PHASE_FADE_IN;
    frame = 0;
    selection = UPGRADE_CHOICE_ACCEPT;
    result = 0;
    for (;;) {
        DrawSync(0);
        BeginFrame();
        switch (phase) {
        case UPGRADE_PHASE_FADE_IN:
            for (i = 1; i < 19; i++) {
                sprites[i].r = frame * 4;
                sprites[i].g = frame * 4;
                sprites[i].b = frame * 4;
            }
            for (i = 0; i < 2; i++) {
                quads[i].r = frame * 4;
                quads[i].g = frame * 4;
                quads[i].b = frame * 4;
            }
            quads[1].r = frame * 2;
            quads[1].g = frame * 2;
            quads[1].b = frame * 2;
            frame++;
            if (frame >= 33) {
                phase++;
            }
            for (i = 1; i < 12; i++) {
                if (i == 11) {
                    GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 4);
                } else {
                    GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 1);
                }
            }
            break;
        case UPGRADE_PHASE_CHOOSE:
            ReadPadEvent();
            switch (gPadEvent) {
            case PAD_EVENT_UP: case PAD_EVENT_UP_LEFT: case PAD_EVENT_UP_RIGHT:
                selection = UPGRADE_CHOICE_ACCEPT;
                break;
            case PAD_EVENT_DOWN: case PAD_EVENT_DOWN_LEFT: case PAD_EVENT_DOWN_RIGHT:
                selection = UPGRADE_CHOICE_DECLINE;
                break;
            case PAD_EVENT_CIRCLE:
                frame = 0;
                progress = 0;
                cycle = 0;
                if (selection == UPGRADE_CHOICE_ACCEPT) {
                    result = 1;
                    phase++;
                } else {
                    phase += 2;
                }
                break;
            }
            if (selection == UPGRADE_CHOICE_ACCEPT) {
                sprites[1].cx = 320;
                sprites[2].cx = 336;
                sprites[9].cx = 336;
                sprites[10].cx = 320;
            } else {
                sprites[2].cx = 320;
                sprites[1].cx = 336;
                sprites[10].cx = 336;
                sprites[9].cx = 320;
            }
            for (i = 1; i < 12; i++) {
                if (i == 11) {
                    GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 4);
                } else {
                    GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 1);
                }
            }
            break;
        case UPGRADE_PHASE_PROGRESS:
            for (i = 1; i < 13; i++) {
                switch (i) {
                case 11: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 4); break;
                case 12: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 1); break;
                case 1: case 2: case 8: case 9: case 10: break;
                default: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 2); break;
                }
            }
            if (progress < 20) {
                sprites[frame + 13].x = progress * 8 + 143;
                sprites[frame + 13].y = 196;
                GsSortFastSprite(&sprites[frame + 13], &gSubOt[gActiveBuffer], 0);
            }
            for (i = 5, j = 0; i > 0 && j < progress; i--, j++) {
                sprites[i + 13].x = (progress - j) * 8 + 135;
                sprites[i + 13].y = 196;
                GsSortFastSprite(&sprites[i + 13], &gSubOt[gActiveBuffer], 0);
            }
            if (cycle == 0) {
                if (progress == 20) {
                    frame = 0;
                    phase++;
                }
                if (frame == 5) {
                    frame = 0;
                    progress++;
                } else {
                    ++frame;
                }
            }
            if (++cycle == 3) {
                cycle = 0;
            }
            break;
        case UPGRADE_PHASE_FADE_OUT:
            for (i = 1; i < 19; i++) {
                sprites[i].r = 128 - frame * 4;
                sprites[i].g = 128 - frame * 4;
                sprites[i].b = 128 - frame * 4;
            }
            for (i = 0; i < 2; i++) {
                quads[i].r = 128 - frame * 4;
                quads[i].g = 128 - frame * 4;
                quads[i].b = 128 - frame * 4;
            }
            quads[1].r = 64 - frame * 2;
            quads[1].g = 64 - frame * 2;
            quads[1].b = 64 - frame * 2;
            frame++;
            if (frame >= 33) {
                phase++;
            }
            for (i = 1; i < 13; i++) {
                switch (i) {
                case 11: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 4); break;
                case 12: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 1); break;
                case 1: case 2: case 8: case 9: case 10: break;
                default: GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 2); break;
                }
            }
            for (i = 5, j = 0; i > 0 && j < progress; i--, j++) {
                sprites[i + 13].x = (progress - j) * 8 + 135;
                sprites[i + 13].y = 196;
                GsSortFastSprite(&sprites[i + 13], &gSubOt[gActiveBuffer], 0);
            }
            break;
        case UPGRADE_PHASE_DONE:
            goto done;
        }
        quads[0].rotateY = angle;
        quads[1].rotateY = angle;
        SortRotatedSpriteQuad(&quads[0], &gSubOt[gActiveBuffer], 3);
        SortRotatedSpriteQuad(&quads[1], &gSubOt[gActiveBuffer], 3);
        VSync(0);
        ResetGraph(1);
        angle += 16;
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
        angle %= 4096;
    }
done:
    DrawSync(0);
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    rect.x = 320;
    rect.w = 320;
    rect.y = 0;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    ReadPadEvent();
    return result;
}

