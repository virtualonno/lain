/* The system menu: a 7x7 grid of tiles that flip over to spell Load, About, Change, Save and
 * Exit, its Are you sure? prompt, and the memory card save and load it runs. Also the message
 * window, the tile grid the media player draws, and the SpriteQuad sorters every screen uses. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include "sprite_quad.h"

#include <libetc.h>
#include <libgs.h>

#include "about.h"
#include "loading.h"
#include "cd.h"
#include "graphics.h"
#include "grid_menu.h"
#include "heap.h"
#include "file_parts.h"
#include "lapks_player.h"
#include "memcard.h"
#include "napk.h"
#include "pad.h"
#include "sound.h"
#include "system_menu.h"
#include "text.h"

#include <libcd.h>
#include <libpress.h>

#include <libspu.h>
#include <rand.h>
#include <memory.h>

#include <strings.h>

#include <libsnd.h>
#include <stdio.h>
#include <malloc.h>
#include <libmcrd.h>
#include "card.h"

short gMemCardAcceptPending = 0;
SpriteQuad *gMenuQuads = 0;
short gMenuFrame;
enum MenuItemId {
    MENU_ITEM_NONE = -1,
    MENU_ITEM_LOAD,
    MENU_ITEM_ABOUT,
    MENU_ITEM_CHANGE,
    MENU_ITEM_SAVE,
    MENU_ITEM_EXIT
};
short gMenuItem;

static int sMessageWindowReady = 0;

SpriteQuad gMessageWindowQuads[8];
GsSPRITE gMessageSprite;

void InitMessageWindow(void)
{
    int row;

    for (row = 0; row < 4; row++) {
        gMessageWindowQuads[row * 2].clut = 0x7941;
        gMessageWindowQuads[row * 2].x = 16;
        gMessageWindowQuads[row * 2].y = 153 + row * 16;
        gMessageWindowQuads[row * 2].tpage = 30;
        gMessageWindowQuads[row * 2].u = 0;
        gMessageWindowQuads[row * 2].v = 224;
        gMessageWindowQuads[row * 2].w = 144;
        gMessageWindowQuads[row * 2].h = 16;
        gMessageWindowQuads[row * 2].r = 128;
        gMessageWindowQuads[row * 2].g = 128;
        gMessageWindowQuads[row * 2].b = 128;
        gMessageWindowQuads[row * 2].attribute = 0;
        gMessageWindowQuads[row * 2 + 1].y = 153 + row * 16;
        gMessageWindowQuads[row * 2 + 1].x = 160;
        gMessageWindowQuads[row * 2 + 1].attribute = 0x800000;
        gMessageWindowQuads[row * 2 + 1].clut = 0x7941;
        gMessageWindowQuads[row * 2 + 1].tpage = 30;
        gMessageWindowQuads[row * 2 + 1].u = 0;
        gMessageWindowQuads[row * 2 + 1].v = 224;
        gMessageWindowQuads[row * 2 + 1].w = 144;
        gMessageWindowQuads[row * 2 + 1].h = 16;
        gMessageWindowQuads[row * 2 + 1].r = 128;
        gMessageWindowQuads[row * 2 + 1].g = 128;
        gMessageWindowQuads[row * 2 + 1].b = 128;
    }

    gMessageSprite.attribute = 0;
    gMessageSprite.x = 52;
    gMessageSprite.y = 156;
    gMessageSprite.tpage = 30;
    gMessageSprite.cy = 485;
    gMessageSprite.cx = 0;
    gMessageSprite.r = 128;
    gMessageSprite.g = 128;
    gMessageSprite.b = 128;
}

void DrawMessageWindow(int message)
{
    GsSPRITE *sprite;
    int i, rows;

    if (sMessageWindowReady == 0) {
        InitMessageWindow();
        sMessageWindowReady = 1;
    }
    sprite = &gMessageSprite;
    sprite->u = gMessageImages[message - 1].u;
    sprite->v = gMessageImages[message - 1].v;
    sprite->w = gMessageImages[message - 1].width;
    sprite->h = gMessageImages[message - 1].height;
    sprite->x = 160 - sprite->w / 2;
    GsSortFastSprite(sprite, &gSubOt[gActiveBuffer], 0);
    rows = gMessageImages[message - 1].rows;
    for (i = 0; i < rows; ++i) {
        SortSpriteQuad(&gMessageWindowQuads[2 * i], &gSubOt[gActiveBuffer], 0);
        SortSpriteQuad(&gMessageWindowQuads[2 * i + 1], &gSubOt[gActiveBuffer], 0);
    }
}

void SetMenuTileImage(int index)
{
    int slot = index + 23;

    gMenuQuads[slot].clut = 0x78c0;
    gMenuQuads[slot].w = 16;
    gMenuQuads[slot].h = 16;
    gMenuQuads[slot].tpage = 26;
    if (gTileNumbers[index] < 13) {
        gMenuQuads[slot].u = (gTileNumbers[index] - 1) * 16;
        gMenuQuads[slot].v = 200;
    } else {
        gMenuQuads[slot].u = (gTileNumbers[index] - 13) * 16;
        gMenuQuads[slot].v = 216;
    }
    if (gTileMirror[index][0] != 0) {
        gMenuQuads[slot].attribute |= 0x400000;
    }
    if (gTileMirror[index][1] != 0) {
        gMenuQuads[slot].attribute |= 0x800000;
    }
}

void SetTileGridImage(int index)
{
    int slot = index;

    gMenuQuads[slot].clut = 0x78c0;
    gMenuQuads[slot].w = 16;
    gMenuQuads[slot].h = 16;
    gMenuQuads[slot].tpage = 26;
    if (gTileNumbers[index] < 13) {
        gMenuQuads[slot].u = (gTileNumbers[index] - 1) * 16;
        gMenuQuads[slot].v = 200;
    } else {
        gMenuQuads[slot].u = (gTileNumbers[index] - 13) * 16;
        gMenuQuads[slot].v = 216;
    }
    if (gTileMirror[index][0] != 0) {
        gMenuQuads[slot].attribute |= 0x400000;
    }
    if (gTileMirror[index][1] != 0) {
        gMenuQuads[slot].attribute |= 0x800000;
    }
}

short gPromptWidth;
short gYesWidth;
short gNoWidth;
short gVersionWidth;
short gPermissionWidth;
short gDeniedWidth;

void InitMenuQuads(void)
{
    volatile int i;
    int slot;

    memset(gMenuQuads, 0, 128 * sizeof(*gMenuQuads));
    for (i = 0; i < 128; i++) {
        gMenuQuads[i].r = 128;
        gMenuQuads[i].g = 128;
        gMenuQuads[i].b = 128;
        gMenuQuads[i].scale[0] = 4096;
        gMenuQuads[i].scale[1] = 4096;
    }
    for (i = 0; i < 49; i++) {
        slot = i + 72;
        gMenuQuads[slot].x = 153;
        gMenuQuads[slot].y = 97;
        gMenuQuads[slot].clut = 0x78c1;
        gMenuQuads[slot].tpage = 26;
        gMenuQuads[slot].w = 14;
        gMenuQuads[slot].h = 14;
        gMenuQuads[slot].u = 0;
        gMenuQuads[slot].v = 232;
        if (gTileMirror[i][0] != 0) {
            gMenuQuads[slot].attribute |= 0x800000;
        }
        if (gTileMirror[i][1] != 0) {
            gMenuQuads[slot].attribute |= 0x400000;
        }
        gMenuQuads[slot].rotateX = 0;
        gMenuQuads[slot].rotateY = 1024;
        gMenuQuads[slot].rotateZ = 1024;
    }
    for (i = 0; i < 49; i++) {
        slot = i + 23;
        gMenuQuads[slot].x = 152;
        gMenuQuads[slot].y = 96;
        SetMenuTileImage(i);
        gMenuQuads[slot].rotateX = 0;
        gMenuQuads[slot].rotateY = 1024;
        gMenuQuads[slot].rotateZ = 1024;
    }

    gMenuQuads[0].clut = 0x7841;
    gMenuQuads[0].x = 0x53;
    gMenuQuads[0].y = 108;
    gMenuQuads[0].tpage = 0x1b;
    gMenuQuads[0].u = 0;
    gMenuQuads[0].v = 112;
    gMenuQuads[0].w = gPromptWidth;
    gMenuQuads[0].h = 0x18;
    gMenuQuads[1].clut = 0x78c1;
    gMenuQuads[1].x = 0;
    gMenuQuads[1].y = 0x69;
    gMenuQuads[1].tpage = 0x1a;
    gMenuQuads[1].u = 0x40;
    gMenuQuads[1].v = 224;
    gMenuQuads[1].w = 0xa0;
    gMenuQuads[1].h = 0x19;
    gMenuQuads[1].attribute = 0x40000000;
    gMenuQuads[2].clut = 0x78c1;
    gMenuQuads[2].x = 0xa0;
    gMenuQuads[2].y = 0x69;
    gMenuQuads[2].tpage = 0x1a;
    gMenuQuads[2].u = 0x40;
    gMenuQuads[2].v = 224;
    gMenuQuads[2].w = 0xa0;
    gMenuQuads[2].h = 0x19;
    gMenuQuads[2].attribute = 0x40800000;
    gMenuQuads[3].clut = 0x7841;
    gMenuQuads[3].x = 0x28;
    gMenuQuads[3].y = 0x8f;
    gMenuQuads[3].tpage = 0x1b;
    gMenuQuads[3].u = 0;
    gMenuQuads[3].v = 136;
    gMenuQuads[3].w = gYesWidth;
    gMenuQuads[3].h = 0x18;
    gMenuQuads[4].clut = 0x7841;
    gMenuQuads[4].x = 0xf3;
    gMenuQuads[4].y = 0x8f;
    gMenuQuads[4].tpage = 0x1b;
    gMenuQuads[4].u = 0;
    gMenuQuads[4].v = 160;
    gMenuQuads[4].w = gNoWidth;
    gMenuQuads[4].h = 0x18;
    gMenuQuads[5].x = 0xe6;
    gMenuQuads[5].clut = 0x78c1;
    gMenuQuads[5].y = 0x8c;
    gMenuQuads[5].tpage = 0x1a;
    gMenuQuads[5].u = 192;
    gMenuQuads[5].v = 200;
    gMenuQuads[5].w = 0x33;
    gMenuQuads[5].h = 0x19;
    gMenuQuads[5].attribute = 0x40000000;
    gMenuQuads[6].clut = 0x78c1;
    gMenuQuads[6].y = 0xd1;
    gMenuQuads[6].x = 0x8c;
    gMenuQuads[6].tpage = 0x1a;
    gMenuQuads[6].u = 194;
    gMenuQuads[6].v = 202;
    gMenuQuads[6].h = 0xa;
    gMenuQuads[6].scale[0] = 0x7000;
    gMenuQuads[6].scale[1] = 0x5000;
    gMenuQuads[6].w = 0x28;
    gMenuQuads[6].attribute = 0x40000000;
    gMenuQuads[7].clut = 0x7840;
    gMenuQuads[7].x = 160 - gVersionWidth / 2;
    gMenuQuads[7].y = 0xc8;
    gMenuQuads[7].tpage = 0x1b;
    gMenuQuads[7].u = 0;
    gMenuQuads[7].v = 0x58;
    gMenuQuads[7].w = 0x100;
    gMenuQuads[7].h = 0x10;
    gMenuQuads[8].clut = 0x7841;
    gMenuQuads[8].x = 160 - (gPermissionWidth + gDeniedWidth) / 2;
    gMenuQuads[8].y = 108;
    gMenuQuads[8].tpage = 0x1b;
    gMenuQuads[8].u = 0;
    gMenuQuads[8].v = 0x40;
    gMenuQuads[8].w = gPermissionWidth;
    gMenuQuads[8].h = 0x18;
    gMenuQuads[9].clut = 0x7841;
    gMenuQuads[9].y = 108;
    gMenuQuads[9].tpage = 0x1b;
    gMenuQuads[9].x = 160 - (gPermissionWidth + gDeniedWidth) / 2 + gPermissionWidth;
    gMenuQuads[9].u = 0;
    gMenuQuads[9].v = 0x28;
    gMenuQuads[9].w = gDeniedWidth;
    gMenuQuads[9].h = 0x18;
}

u_char gSiteLabel[] = "Site";
u_char gSaveLabel[] = "Save";
u_char gExitLabel[] = "Exit";
u_char gLoadLabel[] = "Load";
u_char gAboutLabel[] = "About";
u_char gChangeLabel[] = "Change";
u_char gYesLabel[] = "Yes";
u_char gNoLabel[] = "No";
u_char gDeniedLabel[] = "denied";

void RenderMenuText(void)
{
    char versionText[80];

    UploadTextToVram(gSiteLabel, 704, 440, TEXT_UPLOAD_LARGE, 1);
    UploadTextToVram(gSaveLabel, 704, 464, TEXT_UPLOAD_LARGE, 1);
    UploadTextToVram(gExitLabel, 704, 488, TEXT_UPLOAD_LARGE, 1);
    UploadTextToVram(gLoadLabel, 728, 440, TEXT_UPLOAD_LARGE, 1);
    UploadTextToVram(gAboutLabel, 728, 464, TEXT_UPLOAD_LARGE, 1);
    UploadTextToVram(gChangeLabel, 728, 488, TEXT_UPLOAD_LARGE, 1);
    gPromptWidth = UploadTextToVram((u_char *)"Are you sure?", 704, 368, TEXT_UPLOAD_LARGE, 0);
    gYesWidth = UploadTextToVram(gYesLabel, 704, 392, TEXT_UPLOAD_LARGE, 0);
    gNoWidth = UploadTextToVram(gNoLabel, 704, 416, TEXT_UPLOAD_LARGE, 0);
    sprintf(versionText, "Application Version %d.0", gUpgradesApplied + 1);
    gVersionWidth = UploadTextToVram((u_char *)versionText, 704, 344, TEXT_UPLOAD_SMALL, 1);
    gPermissionWidth = UploadTextToVram((u_char *)"Permission ", 704, 320, TEXT_UPLOAD_LARGE, 0);
    gDeniedWidth = UploadTextToVram(gDeniedLabel, 704, 296, TEXT_UPLOAD_LARGE, 0);
}

void DrawSystemMenuExcept(int first, int second)
{
    u_char shown[49];
    int col;
    int count;
    int i;
    memset(shown, 1, 49);
    if (first != MENU_ITEM_NONE) {
        col = (u_char)(gMenuItems[first].start % 7);
        count = gMenuItems[first].count;
        if (col + count > 7) {
            count = 7 - col;
        }
        for (i = 0; i < count; i++) {
            shown[gMenuItems[first].start + i] = 0;
        }
    }
    if (second != MENU_ITEM_NONE) {
        col = (u_char)(gMenuItems[second].start % 7);
        count = gMenuItems[second].count;
        if (col + count > 7) {
            count = 7 - col;
        }
        for (i = 0; i < count; i++) {
            shown[gMenuItems[second].start + i] = 0;
        }
    }
    for (i = 0; i < 49; i++) {
        if (shown[i] != 0) {
            SortSpriteQuad(&gMenuQuads[i + 23], &gSubOt[gActiveBuffer], i + 23);
        }
        SortSpriteQuad(&gMenuQuads[i + 72], &gSubOt[gActiveBuffer], i + 72);
    }
    SortRotatedSpriteQuad(&gMenuQuads[7], &gSubOt[gActiveBuffer], 4);
    SortRotatedSpriteQuad(&gMenuQuads[6], &gSubOt[gActiveBuffer], 4);
}

short gOverflowBank;

enum MenuState {
    MENU_CHOOSE, MENU_CONFIRM, MENU_MOVE, MENU_CONFIRM_OPEN, MENU_CONFIRM_CLOSE,
    MENU_SAVE, MENU_LOAD, MENU_CHANGE, MENU_DENIED
};
short gMenuState;

void DrawSystemMenu(void)
{
    int i;
    int overflow;
    int offset;

    for (i = 0; i < 49; i++) {
        SortSpriteQuad(&gMenuQuads[i + 23], &gSubOt[gActiveBuffer], i + 23);
        SortSpriteQuad(&gMenuQuads[i + 72], &gSubOt[gActiveBuffer], i + 72);
    }
    if (gMenuItem != MENU_ITEM_NONE) {
        overflow = (u_char)(gMenuItems[gMenuItem].start % 7) + gMenuItems[gMenuItem].count - 7;
        offset = (gOverflowBank == 0) * 4;
        for (i = 0; i < overflow; i++) {
            SortSpriteQuad(&gMenuQuads[i + 15 + offset], &gSubOt[gActiveBuffer], i + 15 + offset);
        }
    }
    if (gMenuState == MENU_CONFIRM) {
        SortSpriteQuad(&gMenuQuads[0], &gSubOt[gActiveBuffer], 1);
        SortSpriteQuad(&gMenuQuads[1], &gSubOt[gActiveBuffer], 2);
        SortSpriteQuad(&gMenuQuads[2], &gSubOt[gActiveBuffer], 2);
        SortSpriteQuad(&gMenuQuads[3], &gSubOt[gActiveBuffer], 3);
        SortSpriteQuad(&gMenuQuads[4], &gSubOt[gActiveBuffer], 3);
        SortSpriteQuad(&gMenuQuads[5], &gSubOt[gActiveBuffer], 4);
    }
    if (gMenuState == MENU_DENIED) {
        SortSpriteQuad(&gMenuQuads[8], &gSubOt[gActiveBuffer], 1);
        SortSpriteQuad(&gMenuQuads[9], &gSubOt[gActiveBuffer], 1);
        SortSpriteQuad(&gMenuQuads[1], &gSubOt[gActiveBuffer], 2);
        SortSpriteQuad(&gMenuQuads[2], &gSubOt[gActiveBuffer], 2);
    }
    SortRotatedSpriteQuad(&gMenuQuads[7], &gSubOt[gActiveBuffer], 5);
    SortRotatedSpriteQuad(&gMenuQuads[6], &gSubOt[gActiveBuffer], 5);
}

short gFlipToLabelFrame = 0;
short gFlipToTilesFrame = 0;

int FlipItemToLabel(int item)
{
    volatile int i;
    u_char first;
    int overflow;
    int col;
    int slot;
    int x, y;
    int rowBase;
    int step;
    int cell;

    if (gFlipToLabelFrame == 9) {
        first = gMenuItems[item].start;
        overflow = (u_char)(first % 7) + gMenuItems[item].count - 7;
        if (overflow < 0) {
            overflow = 0;
        }
        x = 248;
        y = (u_char)(first / 7) * 24 + 24;
        for (i = 0; i < overflow; i++, x += 24) {
            rowBase = 4 * gOverflowBank + 15;
            slot = i + rowBase;
            gMenuQuads[slot].w = 16;
            gMenuQuads[slot].h = 24;
            gMenuQuads[slot].x = x;
            gMenuQuads[slot].y = y;
            gMenuQuads[slot].clut = 0x7842;
            gMenuQuads[slot].tpage = getTPage(0, 0, 704, 512 - 24 * gMenuItems[item].labelRow);
            gMenuQuads[slot].u = 96 * gMenuItems[item].labelColumn + 16 * (gMenuItems[item].count - overflow + i);
            gMenuQuads[slot].v = -24 * gMenuItems[item].labelRow;
        }
        for (i = 0; i < gMenuItems[item].count - overflow; i++) {
            if (i == 0) {
                gMenuQuads[i + gMenuItems[item].start + 23].clut = 0x7841;
            } else {
                gMenuQuads[i + gMenuItems[item].start + 23].clut = 0x7842;
            }
            gMenuQuads[i + gMenuItems[item].start + 23].tpage = GetTPage(0, 0, 704, 512 - 24 * gMenuItems[item].labelRow);
            gMenuQuads[i + gMenuItems[item].start + 23].u = 96 * gMenuItems[item].labelColumn + 16 * i;
            gMenuQuads[i + gMenuItems[item].start + 23].v = -24 * gMenuItems[item].labelRow;
            gMenuQuads[i + gMenuItems[item].start + 23].attribute = 0;
        }
    }
    for (i = 0; i < gMenuItems[item].count; i++) {
        unsigned int start = gMenuItems[item].start;
        col = (u_char)(start % 7);
        if (col + i >= 7) {
            step = i;
            step += 15;
            cell = col;
            cell += step;
            rowBase = 4 * gOverflowBank - 7;
            slot = cell + rowBase;
        } else {
            cell = i;
            rowBase = start + 23;
            slot = cell + rowBase;
        }
        if (gFlipToLabelFrame < 9) {
            gMenuQuads[slot].rotateY = gFlipToLabelFrame * 256;
        } else {
            gMenuQuads[slot].rotateY = 2048 - (gFlipToLabelFrame - 8) * 256;
        }
        if (gFlipToLabelFrame != 8) {
            gMenuQuads[slot].rotateX += 256;
            gMenuQuads[slot].rotateX %= 4096;
        }
        if (gFlipToLabelFrame >= 9 || slot >= 23) {
            SortRotatedSpriteQuad(&gMenuQuads[slot], &gSubOt[gActiveBuffer], slot);
        }
    }
    if (gFlipToLabelFrame == 16) {
        gFlipToLabelFrame = 0;
        return 1;
    }
    gFlipToLabelFrame++;
    return 0;
}

int FlipItemToTiles(int item)
{
    volatile int i;
    int n;
    u_char first;
    int col;
    int idx;
    int offset;
    int pos;

    if (gFlipToTilesFrame == 9) {
        first = gMenuItems[item].start;
        n = first % 7 + gMenuItems[item].count - 7;
        if (n < 0) {
            n = 0;
        }
        i = 1;
        gMenuQuads[first + 23].clut = 0x7840;
        for (; i < gMenuItems[item].count - n; i++) {
            SetMenuTileImage(gMenuItems[item].start + i);
        }
    }
    for (i = 0; i < gMenuItems[item].count; i++) {
        first = gMenuItems[item].start;
        col = first % 7;
        if (col + i >= 7) {
            int step;
            offset = (gOverflowBank == 0) * 4;
            step = i + 15;
            pos = col + step;
            offset -= 7;
            idx = pos + offset;
        } else {
            pos = i;
            offset = first + 23;
            idx = pos + offset;
        }
        if (gFlipToTilesFrame < 9) {
            gMenuQuads[idx].rotateY = gFlipToTilesFrame * 256;
        } else {
            gMenuQuads[idx].rotateY = 0x800 - (gFlipToTilesFrame - 8) * 256;
        }
        if (gFlipToTilesFrame != 8) {
            gMenuQuads[idx].rotateX += 0x100;
            gMenuQuads[idx].rotateX = gMenuQuads[idx].rotateX % 0x1000;
        }
        if (gFlipToTilesFrame < 9 || idx >= 23) {
            SortRotatedSpriteQuad(&gMenuQuads[idx], &gSubOt[gActiveBuffer], idx);
        }
    }
    if (gFlipToTilesFrame == 16) {
        gFlipToTilesFrame = 0;
        return 1;
    }
    gFlipToTilesFrame++;
    return 0;
}

short gPreviousMenuItem;
short gConfirmYes;
short gMenuAnimating; /* 1 while a move or the Yes/No panel animates; nothing reads it */
short gMenuCounter; /* frames a message stays up, or memory card retries */
enum MemCardSaveStep {
    MEMCARD_SAVE_ACCEPT,
    MEMCARD_SAVE_POLL_ACCEPT,
    MEMCARD_SAVE_OPEN,
    MEMCARD_SAVE_SHOW_OVERWRITE,
    MEMCARD_SAVE_CONFIRM_OVERWRITE,
    MEMCARD_SAVE_START_WRITE,
    MEMCARD_SAVE_POLL_WRITE,
    MEMCARD_SAVE_DONE,
    MEMCARD_SAVE_WAIT_OVERWRITE_SYNC,
    MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT,
    MEMCARD_SAVE_ERR_NO_CARD,
    MEMCARD_SAVE_ERR_INVALID = 20,
    MEMCARD_SAVE_SHOW_FORMAT = 30,
    MEMCARD_SAVE_CONFIRM_FORMAT,
    MEMCARD_SAVE_FORMAT,
    MEMCARD_SAVE_FORMAT_CHECK_INIT = 35,
    MEMCARD_SAVE_FORMAT_CHECK_ACCEPT,
    MEMCARD_SAVE_FORMAT_CHECK_POLL,
    MEMCARD_SAVE_WAIT_FORMAT_SYNC,
    MEMCARD_SAVE_RETRY_FORMAT_ACCEPT,
    MEMCARD_SAVE_CREATE,
    MEMCARD_SAVE_OPEN_CREATED,
    MEMCARD_SAVE_ERR_FULL = 50,
    MEMCARD_SAVE_CANCEL_SYNC = 90
};
short gMemCardStep;
CardSaveBlock *gMemCardBuffer;

int StepSystemMenuOpen(void)
{
    volatile int i;
    volatile int frame;
    short group;
    int u, row;
    int slot;

    if (gMenuFrame == 0) {
        gMenuQuads = (SpriteQuad *)AllocHeapBlock(0x2600);
        if (gMenuQuads == 0) {
            return 0;
        }
        RenderMenuText();
        InitMenuQuads();
    }
    if (gMenuQuads == 0) {
        return 0;
    }
    if (gMenuFrame < 17) {
        for (i = 0; i < 49; i++) {
            gMenuQuads[i + 23].x = (24 * (i % 7) - 72) * gMenuFrame / 16 + 152;
            gMenuQuads[i + 23].y = (24 * (i / 7) - 72) * gMenuFrame / 16 + 96;
            gMenuQuads[i + 72].x = (21 * (i % 7) - 63) * gMenuFrame / 16 + 153;
            gMenuQuads[i + 72].y = (21 * (i / 7) - 63) * gMenuFrame / 16 + 97;
            if (gMenuFrame != 0) {
                gMenuQuads[i + 23].rotateY -= 64;
                gMenuQuads[i + 23].rotateZ -= 64;
                gMenuQuads[i + 72].rotateY -= 64;
                gMenuQuads[i + 72].rotateZ -= 64;
            }
            SortRotatedSpriteQuad(&gMenuQuads[i + 23], &gSubOt[gActiveBuffer], i + 23);
            SortRotatedSpriteQuad(&gMenuQuads[i + 72], &gSubOt[gActiveBuffer], i + 72);
        }
        gMenuFrame++;
    } else if (gMenuFrame < 82) {
        group = (gMenuFrame - 17) / 4;
        for (i = 0; i < 49; i++) {
            frame = gMenuFrame - (short)(4 * gTileFlipDelay[i] + 17);
            if (group >= (short)gTileFlipDelay[i] && frame >= 0 && frame < 17) {
                if (frame < 9) {
                    gMenuQuads[i + 23].rotateY = frame * 256;
                } else {
                    gMenuQuads[i + 23].rotateY = 2048 - (frame - 8) * 256;
                    if (frame == 9) {
                        switch (i) {
                        case 8:
                        case 19:
                        case 24:
                        case 36:
                        case 47:
                            switch (i) {
                            case 8:  u = 96; row = 3; break;
                            case 19: u = 96; row = 2; break;
                            case 24: u = 96; row = 1; break;
                            case 36: u = 0;  row = 2; break;
                            case 47: u = 0;  row = 1; break;
                            }
                            slot = i + 23;
                            gMenuQuads[slot].clut = 0x7840;
                            gMenuQuads[slot].tpage = GetTPage(0, 0, 704, 512 - row * 24);
                            gMenuQuads[slot].u = u;
                            gMenuQuads[slot].v = -(row * 24);
                            gMenuQuads[slot].attribute = 0;
                            break;
                        }
                    }
                }
                if (frame != 8) {
                    gMenuQuads[i + 23].rotateX += 256;
                    gMenuQuads[i + 23].rotateX %= 4096;
                }
            }
            SortRotatedSpriteQuad(&gMenuQuads[i + 23], &gSubOt[gActiveBuffer], i + 23);
            SortSpriteQuad(&gMenuQuads[i + 72], &gSubOt[gActiveBuffer], i + 72);
        }
        gMenuFrame++;
    } else if (gMenuFrame < 100) {
        DrawSystemMenuExcept(MENU_ITEM_CHANGE, MENU_ITEM_NONE);
        if (FlipItemToLabel(MENU_ITEM_CHANGE) != 0) {
            gMenuAnimating = 0;
            gOverflowBank = 1;
            gMenuState = MENU_CHOOSE;
            gMenuFrame = 0;
            return 1;
        }
        gOverflowBank = 0;
        gMenuItem = MENU_ITEM_CHANGE;
    }
    return 0;
}

int StepConfirmOpen(void)
{
    RECT unused; /* kept: nothing reads it; without it the routine compiles differently */

    SortSpriteQuad(&gMenuQuads[0], &gSubOt[gActiveBuffer], 0);
    SortSpriteQuad(&gMenuQuads[1], &gSubOt[gActiveBuffer], 1);
    SortSpriteQuad(&gMenuQuads[2], &gSubOt[gActiveBuffer], 1);
    SortSpriteQuad(&gMenuQuads[3], &gSubOt[gActiveBuffer], 2);
    SortSpriteQuad(&gMenuQuads[4], &gSubOt[gActiveBuffer], 2);
    SortSpriteQuad(&gMenuQuads[5], &gSubOt[gActiveBuffer], 3);
    if (gMenuFrame == 8) {
        gMenuFrame = 0;
        return 1;
    }
    gMenuFrame++;
    return 0;
}

int StepConfirmClose(void)
{
    RECT unused; /* kept: nothing reads it; without it the routine compiles differently */

    SortSpriteQuad(&gMenuQuads[0], &gSubOt[gActiveBuffer], 0);
    SortSpriteQuad(&gMenuQuads[1], &gSubOt[gActiveBuffer], 1);
    SortSpriteQuad(&gMenuQuads[2], &gSubOt[gActiveBuffer], 1);
    SortSpriteQuad(&gMenuQuads[3], &gSubOt[gActiveBuffer], 2);
    SortSpriteQuad(&gMenuQuads[4], &gSubOt[gActiveBuffer], 2);
    SortSpriteQuad(&gMenuQuads[5], &gSubOt[gActiveBuffer], 3);
    if (gMenuFrame == 8) {
        gMenuFrame = 0;
        return 1;
    }
    gMenuFrame++;
    return 0;
}

int UpdateSystemMenu(void)
{
    int result;

    ReadPadEvent();
    switch (gMenuState) {
    case MENU_CHOOSE:
        switch (gPadEvent) {
        case PAD_EVENT_UP:
        case PAD_EVENT_UP_LEFT:
        case PAD_EVENT_UP_RIGHT:
            PlaySound(1);
            result = 1;
            break;
        case PAD_EVENT_DOWN:
        case PAD_EVENT_DOWN_LEFT:
        case PAD_EVENT_DOWN_RIGHT:
            PlaySound(1);
            result = -1;
            break;
        case PAD_EVENT_CIRCLE:
            result = 2;
            PlaySound(0);
            break;
        default:
            result = 0;
            break;
        }
        if (result == 1) {
            if (gMenuItem > MENU_ITEM_LOAD) {
                gPreviousMenuItem = gMenuItem;
                gMenuItem--;
                FlipItemToTiles(gPreviousMenuItem);
                FlipItemToLabel(gMenuItem);
                DrawSystemMenuExcept(gPreviousMenuItem, gMenuItem);
                gMenuAnimating = 1;
                gMenuState = MENU_MOVE;
                PlaySound(1);
                break;
            }
        } else if (result < 0) {
            if (gMenuItem < MENU_ITEM_EXIT) {
                gPreviousMenuItem = gMenuItem;
                gMenuItem++;
                FlipItemToTiles(gPreviousMenuItem);
                FlipItemToLabel(gMenuItem);
                DrawSystemMenuExcept(gPreviousMenuItem, gMenuItem);
                gMenuAnimating = 1;
                gMenuState = MENU_MOVE;
                PlaySound(1);
                break;
            }
        } else if (result == 2) {
            DrawSystemMenu();
            switch (gMenuItem) {
            case MENU_ITEM_ABOUT:
                RunAboutScreen();
                UpdateIdleClips(1);
                break;
            case MENU_ITEM_CHANGE:
                if (gGatesOpened < 4) {
                    gMenuState = MENU_DENIED;
                    gMenuCounter = 60;
                    break;
                }
                /* fall through */
            case MENU_ITEM_LOAD:
            case MENU_ITEM_SAVE:
                gMenuAnimating = 1;
                gMenuState = MENU_CONFIRM_OPEN;
                gConfirmYes = 0;
                gMenuQuads[5].x = 230;
                break;
            case MENU_ITEM_EXIT:
                return MENU_RESULT_EXIT;
            }
            break;
        }
        DrawSystemMenu();
        break;
    case MENU_CONFIRM:
        switch (gPadEvent) {
        case PAD_EVENT_UP_LEFT:
        case PAD_EVENT_LEFT:
        case PAD_EVENT_DOWN_LEFT:
            PlaySound(1);
            result = -1;
            break;
        case PAD_EVENT_UP_RIGHT:
        case PAD_EVENT_RIGHT:
        case PAD_EVENT_DOWN_RIGHT:
            PlaySound(1);
            result = 1;
            break;
        case PAD_EVENT_CIRCLE:
            result = 2;
            PlaySound(26);
            break;
        default:
            result = 0;
            break;
        }
        if (result < 0) {
            if (gConfirmYes == 0) {
                gConfirmYes = 1;
                PlaySound(1);
            }
        } else if (result == 1) {
            if (gConfirmYes != 0) {
                gConfirmYes = 0;
                PlaySound(1);
            }
        }
        if (gConfirmYes != 0) {
            gMenuQuads[5].x = 34;
        } else {
            gMenuQuads[5].x = 230;
        }
        DrawSystemMenu();
        if (result == 2) {
            if (gConfirmYes == 0) {
                gMenuState = MENU_CONFIRM_CLOSE;
                gMenuAnimating = 1;
            } else {
                switch (gMenuItem) {
                case MENU_ITEM_LOAD:
                    gMenuState = MENU_LOAD;
                    break;
                case MENU_ITEM_CHANGE:
                    gMenuState = MENU_CHANGE;
                    break;
                case MENU_ITEM_SAVE:
                    gMenuState = MENU_SAVE;
                    break;
                }
                gMemCardStep = 0;
            }
        }
        break;
    case MENU_MOVE:
        FlipItemToTiles(gPreviousMenuItem);
        if (FlipItemToLabel(gMenuItem) != 0) {
            gMenuAnimating = 0;
            gMenuState = MENU_CHOOSE;
            if (++gOverflowBank == 2) {
                gOverflowBank = 0;
            }
        }
        DrawSystemMenuExcept(gPreviousMenuItem, gMenuItem);
        break;
    case MENU_CONFIRM_OPEN:
        DrawSystemMenu();
        if (StepConfirmOpen() != 0) {
            gMenuState = MENU_CONFIRM;
            gMenuAnimating = 0;
        }
        break;
    case MENU_CONFIRM_CLOSE:
        DrawSystemMenu();
        if (StepConfirmClose() != 0) {
            gMenuState = MENU_CHOOSE;
            gMenuAnimating = 0;
        }
        break;
    case MENU_SAVE:
        switch (gMemCardStep) {
        case MEMCARD_SAVE_ACCEPT:
        case MEMCARD_SAVE_FORMAT_CHECK_ACCEPT:
            AcceptMemCard();
            gMemCardStep++;
            break;
        case MEMCARD_SAVE_POLL_ACCEPT:
            result = PollMemCardSync();
            if (result != 1) {
                switch (-result) {
                case McErrCardNotExist:
                    PlaySound(28);
                    gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                    gMenuCounter = 120;
                    break;
                case McErrCardInvalid:
                    PlaySound(28);
                    gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                    gMenuCounter = 120;
                    break;
                case McErrNewCard:
                    gMemCardStep = MEMCARD_SAVE_ACCEPT;
                    break;
                case McErrNotFormat:
                    gMemCardStep = MEMCARD_SAVE_FORMAT_CHECK_INIT;
                    break;
                }
            } else {
                gMemCardStep++;
            }
            break;
        case MEMCARD_SAVE_OPEN:
            switch (-OpenMemCardFile(1)) {
            case McErrNone:
                gMemCardStep++;
                break;
            case McErrCardNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                gMenuCounter = 120;
                break;
            case McErrCardInvalid:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                gMenuCounter = 120;
                break;
            case McErrNotFormat:
                gMemCardStep = MEMCARD_SAVE_FORMAT_CHECK_INIT;
                break;
            case McErrFileNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_CREATE;
                break;
            }
            break;
        case MEMCARD_SAVE_SHOW_OVERWRITE:
            DrawMessageWindow(MESSAGE_OVERWRITE_PROMPT_OPEN);
            gMemCardStep++;
            break;
        case MEMCARD_SAVE_CONFIRM_OVERWRITE:
            if (gMemCardAcceptPending == 0) {
                AcceptMemCard();
                gMemCardAcceptPending = 1;
            } else {
                result = PollMemCardSync();
                if (result != 0) {
                    gMemCardAcceptPending = 0;
                    if (result != 1) {
                        gMemCardStep = MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT;
                        gMenuCounter = 5;
                        break;
                    }
                }
            }
            if (gPadEvent != PAD_EVENT_NONE) {
                if (gPadEvent == PAD_EVENT_START) {
                    PlaySound(26);
                    if (gMemCardAcceptPending == 0) {
                        DrawMessageWindow(MESSAGE_SAVING);
                        gMemCardStep++;
                        break;
                    } else {
                        gMemCardStep = MEMCARD_SAVE_WAIT_OVERWRITE_SYNC;
                    }
                } else {
                    PlaySound(27);
                    if (gMemCardAcceptPending == 0) {
                        gMenuState = MENU_CHOOSE;
                        CloseMemCardFile();
                    } else {
                        gMemCardStep = MEMCARD_SAVE_CANCEL_SYNC;
                    }
                }
            } else {
                DrawMessageWindow(MESSAGE_OVERWRITE_PROMPT);
            }
            break;
        case MEMCARD_SAVE_START_WRITE:
            gMemCardBuffer = AllocHeapBlock(0x6000);
            WriteMemCardFile(gMemCardBuffer, PackCardSaveBlock(gMemCardBuffer) << 7);
            DrawMessageWindow(MESSAGE_SAVING);
            gMemCardStep++;
            break;
        case MEMCARD_SAVE_POLL_WRITE:
            DrawMessageWindow(MESSAGE_SAVING);
            result = PollMemCardSync();
            if (result != 0) {
                FreeHeapBlock(gMemCardBuffer);
                gMemCardBuffer = 0;
                CloseMemCardFile();
                if (result == 1) {
                    gMenuCounter = 120;
                    gMemCardStep++;
                } else {
                    switch (-result) {
                    case McErrCardNotExist:
                        PlaySound(28);
                        gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                        gMenuCounter = 120;
                        break;
                    case McErrCardInvalid:
                        PlaySound(28);
                        gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                        gMenuCounter = 120;
                        break;
                    case McErrNewCard:
                        gMemCardStep = MEMCARD_SAVE_ACCEPT;
                        break;
                    }
                }
            }
            break;
        case MEMCARD_SAVE_DONE:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_SAVED);
            break;
        case MEMCARD_SAVE_WAIT_OVERWRITE_SYNC:
            result = PollMemCardSync();
            if (result != 0) {
                gMemCardAcceptPending = 0;
                if (result != 1) {
                    gMemCardStep = MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT;
                    gMenuCounter = 5;
                } else {
                    DrawMessageWindow(MESSAGE_SAVING);
                    gMemCardStep = MEMCARD_SAVE_START_WRITE;
                }
            }
            break;
        case MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT:
            if (gMenuCounter > 0) {
                gMenuCounter--;
            }
            if (gMenuCounter == 0) {
                if (gMemCardAcceptPending == 0) {
                    AcceptMemCard();
                    gMemCardAcceptPending = 1;
                } else {
                    result = PollMemCardSync();
                    if (result != 0) {
                        gMemCardAcceptPending = 0;
                        if (result == 1) {
                            gMemCardStep = MEMCARD_SAVE_CONFIRM_OVERWRITE;
                        } else {
                            switch (-result) {
                            case McErrNone:
                                gMemCardStep = MEMCARD_SAVE_CONFIRM_OVERWRITE;
                                break;
                            case McErrCardNotExist:
                                PlaySound(28);
                                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                                gMenuCounter = 120;
                                CloseMemCardFile();
                                break;
                            case McErrCardInvalid:
                                PlaySound(28);
                                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                                gMenuCounter = 120;
                                CloseMemCardFile();
                                break;
                            case McErrNewCard:
                                gMemCardStep = MEMCARD_SAVE_ACCEPT;
                                CloseMemCardFile();
                                break;
                            }
                        }
                    }
                }
            }
            break;
        case MEMCARD_SAVE_ERR_NO_CARD:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_NO_CARD);
            break;
        case MEMCARD_SAVE_ERR_INVALID:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_INVALID_CARD);
            break;
        case MEMCARD_SAVE_SHOW_FORMAT:
            DrawMessageWindow(MESSAGE_FORMAT_PROMPT);
            gMemCardStep++;
            break;
        case MEMCARD_SAVE_CONFIRM_FORMAT:
            if (gMemCardAcceptPending == 0) {
                AcceptMemCard();
                gMemCardAcceptPending = 1;
            } else {
                result = PollMemCardSync();
                if (result != 0) {
                    gMemCardAcceptPending = 0;
                    if (result != -McErrNotFormat) {
                        gMemCardStep = MEMCARD_SAVE_RETRY_FORMAT_ACCEPT;
                        gMenuCounter = 5;
                        break;
                    }
                }
            }
            if (gPadEvent != PAD_EVENT_NONE) {
                if (gPadEvent == PAD_EVENT_START) {
                    PlaySound(26);
                    if (gMemCardAcceptPending == 0) {
                        DrawMessageWindow(MESSAGE_FORMATTING);
                        gMemCardStep++;
                        break;
                    } else {
                        gMemCardStep = MEMCARD_SAVE_WAIT_FORMAT_SYNC;
                    }
                } else {
                    PlaySound(27);
                    if (gMemCardAcceptPending == 0) {
                        gMenuState = MENU_CHOOSE;
                    } else {
                        gMemCardStep = MEMCARD_SAVE_CANCEL_SYNC;
                    }
                }
            } else {
                DrawMessageWindow(MESSAGE_FORMAT_PROMPT);
            }
            break;
        case MEMCARD_SAVE_FORMAT:
            switch (-FormatMemCard()) {
            case McErrNone:
                gMemCardStep = MEMCARD_SAVE_CREATE;
                DrawMessageWindow(MESSAGE_FORMATTED);
                break;
            case McErrCardNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                gMenuCounter = 120;
                break;
            case McErrCardInvalid:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                gMenuCounter = 120;
                break;
            }
            break;
        case MEMCARD_SAVE_FORMAT_CHECK_INIT:
            gMenuCounter = 0;
            gMemCardStep++;
            break;
        case MEMCARD_SAVE_FORMAT_CHECK_POLL:
            result = PollMemCardSync();
            if (result != 0) {
                if (result == 1) {
                    gMemCardStep = MEMCARD_SAVE_ACCEPT;
                } else {
                    switch (-result) {
                    case McErrCardNotExist:
                        PlaySound(28);
                        gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                        gMenuCounter = 120;
                        break;
                    case McErrCardInvalid:
                        PlaySound(28);
                        gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                        gMenuCounter = 120;
                        break;
                    case McErrNewCard:
                        gMemCardStep = MEMCARD_SAVE_ACCEPT;
                        break;
                    case McErrNotFormat:
                        if (++gMenuCounter == 5) {
                            PlaySound(28);
                            gMemCardStep = MEMCARD_SAVE_SHOW_FORMAT;
                        } else {
                            gMemCardStep = MEMCARD_SAVE_FORMAT_CHECK_ACCEPT;
                        }
                        break;
                    }
                }
            }
            break;
        case MEMCARD_SAVE_WAIT_FORMAT_SYNC:
            result = PollMemCardSync();
            if (result != 0) {
                gMemCardAcceptPending = 0;
                if (result == -McErrNotFormat) {
                    DrawMessageWindow(MESSAGE_FORMATTING);
                    gMemCardStep = MEMCARD_SAVE_FORMAT;
                } else {
                    gMemCardStep = MEMCARD_SAVE_RETRY_FORMAT_ACCEPT;
                    gMenuCounter = 5;
                }
            }
            break;
        case MEMCARD_SAVE_RETRY_FORMAT_ACCEPT:
            if (gMenuCounter > 0) {
                gMenuCounter--;
            }
            if (gMenuCounter == 0) {
                if (gMemCardAcceptPending == 0) {
                    AcceptMemCard();
                    gMemCardAcceptPending = 1;
                } else {
                    result = PollMemCardSync();
                    if (result != 0) {
                        gMemCardAcceptPending = 0;
                        if (result == 1) {
                            gMemCardStep = MEMCARD_SAVE_CONFIRM_FORMAT;
                        } else {
                            switch (-result) {
                            case McErrNone:
                                gMemCardStep = MEMCARD_SAVE_CONFIRM_FORMAT;
                                break;
                            case McErrCardNotExist:
                                PlaySound(28);
                                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                                gMenuCounter = 120;
                                CloseMemCardFile();
                                break;
                            case McErrCardInvalid:
                                PlaySound(28);
                                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                                gMenuCounter = 120;
                                CloseMemCardFile();
                                break;
                            case McErrNewCard:
                                gMemCardStep = MEMCARD_SAVE_ACCEPT;
                                CloseMemCardFile();
                                break;
                            }
                        }
                    }
                }
            }
            break;
        case MEMCARD_SAVE_CREATE:
            switch (-CreateMemCardFile()) {
            case McErrNone:
                gMemCardStep++;
                break;
            case McErrCardNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                gMenuCounter = 120;
                break;
            case McErrCardInvalid:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                gMenuCounter = 120;
                break;
            case McErrNotFormat:
                gMemCardStep = MEMCARD_SAVE_FORMAT_CHECK_INIT;
                break;
            case McErrBlockFull:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_FULL;
                gMenuCounter = 120;
                break;
            }
            break;
        case MEMCARD_SAVE_OPEN_CREATED:
            switch (-OpenMemCardFile(1)) {
            case McErrNone:
                gMemCardStep = MEMCARD_SAVE_START_WRITE;
                break;
            case McErrCardNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_NO_CARD;
                gMenuCounter = 120;
                break;
            case McErrCardInvalid:
                PlaySound(28);
                gMemCardStep = MEMCARD_SAVE_ERR_INVALID;
                gMenuCounter = 120;
                break;
            case McErrNotFormat:
                gMemCardStep = MEMCARD_SAVE_FORMAT_CHECK_INIT;
                break;
            case McErrFileNotExist:
                gMemCardStep = MEMCARD_SAVE_CREATE;
                break;
            }
            break;
        case MEMCARD_SAVE_ERR_FULL:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_NO_FREE_BLOCK);
            break;
        case MEMCARD_SAVE_CANCEL_SYNC:
            if (PollMemCardSync() != 0) {
                CloseMemCardFile();
                gMenuState = MENU_CHOOSE;
                gMemCardAcceptPending = 0;
            }
            break;
        }
        DrawSystemMenu();
        break;
    case MENU_LOAD:
        switch (gMemCardStep) {
        case MEMCARD_LOAD_ACCEPT:
            AcceptMemCard();
            gMemCardStep++;
            break;
        case MEMCARD_LOAD_POLL_ACCEPT:
            result = PollMemCardSync();
            if (result != 1) {
                switch (-result) {
                case McErrCardNotExist:
                    PlaySound(28);
                    gMemCardStep = MEMCARD_LOAD_ERR_NO_CARD;
                    gMenuCounter = 120;
                    break;
                case McErrCardInvalid:
                    PlaySound(28);
                    gMemCardStep = MEMCARD_LOAD_ERR_INVALID;
                    gMenuCounter = 120;
                    break;
                case McErrNewCard:
                    gMemCardStep = MEMCARD_LOAD_ACCEPT;
                    break;
                case McErrNotFormat:
                    PlaySound(28);
                    gMemCardStep = MEMCARD_LOAD_ERR_UNFORMATTED;
                    gMenuCounter = 120;
                    break;
                }
            } else {
                gMemCardStep = MEMCARD_LOAD_OPEN;
            }
            break;
        case MEMCARD_LOAD_OPEN:
            switch (-OpenMemCardFile(0)) {
            case McErrNone:
                gMenuCounter = 10;
                gMemCardStep++;
                break;
            case McErrCardNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_LOAD_ERR_NO_CARD;
                gMenuCounter = 120;
                break;
            case McErrCardInvalid:
                PlaySound(28);
                gMemCardStep = MEMCARD_LOAD_ERR_INVALID;
                gMenuCounter = 120;
                break;
            case McErrNotFormat:
                PlaySound(28);
                gMemCardStep = MEMCARD_LOAD_ERR_UNFORMATTED;
                gMenuCounter = 120;
                break;
            case McErrFileNotExist:
                PlaySound(28);
                gMemCardStep = MEMCARD_LOAD_ERR_NO_FILE;
                gMenuCounter = 120;
                break;
            }
            break;
        case MEMCARD_LOAD_START_READ:
            gMemCardBuffer = AllocHeapBlock(0x6000);
            ReadMemCardFile(gMemCardBuffer, PackCardSaveBlock(gMemCardBuffer) << 7);
            DrawMessageWindow(MESSAGE_LOADING);
            gMemCardStep = MEMCARD_LOAD_POLL_READ;
            break;
        case MEMCARD_LOAD_POLL_READ:
            DrawMessageWindow(MESSAGE_LOADING);
            result = PollMemCardSync();
            if (result != 0) {
                if (result == 1) {
                    if (UnpackCardSaveBlock(gMemCardBuffer) != 0) {
                        gMemCardStep++;
                    } else {
                        gMemCardStep += 2;
                    }
                    gMenuCounter = 120;
                    CloseMemCardFile();
                } else {
                    switch (-result) {
                    case McErrCardNotExist:
                        if (--gMenuCounter == 0) {
                            PlaySound(28);
                            gMemCardStep = MEMCARD_LOAD_ERR_NO_CARD;
                            gMenuCounter = 120;
                        } else {
                            gMemCardStep = MEMCARD_LOAD_ACCEPT;
                        }
                        CloseMemCardFile();
                        FreeHeapBlock(gMemCardBuffer);
                        break;
                    case McErrCardInvalid:
                        PlaySound(28);
                        gMemCardStep = MEMCARD_LOAD_ERR_INVALID;
                        gMenuCounter = 120;
                        FreeHeapBlock(gMemCardBuffer);
                        CloseMemCardFile();
                        break;
                    case McErrNewCard:
                        gMemCardStep = MEMCARD_LOAD_ACCEPT;
                        FreeHeapBlock(gMemCardBuffer);
                        CloseMemCardFile();
                        break;
                    }
                }
            }
            break;
        case MEMCARD_LOAD_DONE:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
                FreeHeapBlock(gMemCardBuffer);
                return MENU_RESULT_LOAD;
            }
            DrawMessageWindow(MESSAGE_LOADED);
            break;
        case MEMCARD_LOAD_ERR_CORRUPT:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
                FreeHeapBlock(gMemCardBuffer);
            }
            DrawMessageWindow(MESSAGE_SAVE_CORRUPT);
            break;
        case MEMCARD_LOAD_ERR_NO_CARD:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_NO_CARD);
            break;
        case MEMCARD_LOAD_ERR_INVALID:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_INVALID_CARD);
            break;
        case MEMCARD_LOAD_ERR_UNFORMATTED:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_UNFORMATTED);
            break;
        case MEMCARD_LOAD_ERR_NO_FILE:
            if (--gMenuCounter == 0) {
                gMenuState = MENU_CHOOSE;
            }
            DrawMessageWindow(MESSAGE_NO_SAVE);
            break;
        }
        DrawSystemMenu();
        break;
    case MENU_CHANGE:
        DrawSystemMenu();
        return MENU_RESULT_CHANGE;
    case MENU_DENIED:
        if (--gMenuCounter == 0) {
            gMenuState = MENU_CHOOSE;
        }
        DrawSystemMenu();
        break;
    }
    return MENU_RESULT_STAY;
}

int StepSystemMenuClose(void)
{
    int i;
    int col;
    int row;

    if (gMenuFrame == 0) {
        if (FlipItemToTiles(gMenuItem) != 0) {
            gMenuFrame++;
        }
        DrawSystemMenuExcept(gMenuItem, MENU_ITEM_NONE);
        return 0;
    }
    if (gMenuFrame > 0) {
        for (i = 0; i < 49; i++) {
            col = i % 7 - 3;
            row = i / 7 - 3;
            if (gMenuFrame != 0) {
                gMenuQuads[i + 23].rotateY += 64;
                gMenuQuads[i + 23].rotateZ += 64;
                gMenuQuads[i + 72].rotateY += 64;
                gMenuQuads[i + 72].rotateZ += 64;
            }
            gMenuQuads[i + 23].x += col * 5;
            gMenuQuads[i + 23].y += row * 5;
            gMenuQuads[i + 72].x += col * 5;
            gMenuQuads[i + 72].y += row * 5;
            SortRotatedSpriteQuad(&gMenuQuads[i + 23], &gSubOt[gActiveBuffer], i + 23);
            SortRotatedSpriteQuad(&gMenuQuads[i + 72], &gSubOt[gActiveBuffer], i + 72);
        }
        if (gMenuFrame == 16) {
            FreeHeapBlock(gMenuQuads);
            gMenuFrame = 0;
            return 1;
        }
        gMenuFrame++;
    }
    return 0;
}

u_short gTileGridDepth;

void InitTileGrid(const char *unused, int depth)
{
    int i;

    gTileGridDepth = depth;
    gMenuFrame = -32768;
    gMenuQuads = (SpriteQuad *)AllocHeapBlock(98 * sizeof(*gMenuQuads));
    if (gMenuQuads != 0) {
        memset(gMenuQuads, 0, 98 * sizeof(*gMenuQuads));
        for (i = 0; i < 49; i++) {
            gMenuQuads[i].x = 152;
            gMenuQuads[i].y = 96;
            gMenuQuads[i].r = 128;
            gMenuQuads[i].g = 128;
            gMenuQuads[i].b = 128;
            gMenuQuads[i].scale[0] = 4096;
            gMenuQuads[i].scale[1] = 4096;
            SetTileGridImage(i);
            gMenuQuads[i + 49].r = 128;
            gMenuQuads[i + 49].g = 128;
            gMenuQuads[i + 49].b = 128;
            gMenuQuads[i + 49].x = 153;
            gMenuQuads[i + 49].y = 97;
            gMenuQuads[i + 49].clut = 0x78c1;
            gMenuQuads[i + 49].w = 14;
            gMenuQuads[i + 49].h = 14;
            gMenuQuads[i + 49].tpage = 26;
            gMenuQuads[i + 49].scale[0] = 4096;
            gMenuQuads[i + 49].scale[1] = 4096;
            gMenuQuads[i + 49].u = 0;
            gMenuQuads[i + 49].v = 232;
        }
    }
}

void DrawTileGrid(void)
{
    int i;
    int row;
    int col;
    int scale;
    SpriteQuad *entry;
    SpriteQuad *inner;

    if (gMenuQuads != 0) {
        for (i = 0; i < 49; i++) {
            scale = gMenuFrame;
            row = i / 7;
            col = i % 7;
            gMenuQuads[i].x = (col * 24 - 72) * scale / 16 + 152;
            gMenuQuads[i].y = (row * 24 - 72) * scale / 16 + 96;
            entry = &gMenuQuads[i];
            inner = entry + 49;
            inner->x = (col * 21 - 63) * scale / 16 + 153;
            inner->y = (row * 21 - 63) * scale / 16 + 97;
            if (scale != 0) {
                entry->rotateY -= 64;
                entry->rotateZ -= 64;
                inner->rotateY -= 64;
                inner->rotateZ -= 64;
            }
            SortRotatedSpriteQuad(&gMenuQuads[i], &gOt[gActiveBuffer], gTileGridDepth);
            SortRotatedSpriteQuad(&gMenuQuads[i + 49], &gOt[gActiveBuffer], gTileGridDepth + 1);
        }
        gMenuFrame = (gMenuFrame + 1) | 0x8000;
    }
}

void LoadMenuTileImage(void)
{
    RECT rect;
    TIM_IMAGE tim;
    void *buffer;
    int request;
    u_long *image;

    buffer = AllocHeapBlock(gBinParts[2].size);
    request = RequestFilePartRead(3, 2, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
        DrawLoadingFrame(0);
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[2].size);
    free(buffer);
    OpenTIM(image);
    ReadTIM(&tim);
    LoadClut(tim.caddr, 0, 483);
    setRECT(&rect, 640, 456, 62, 48);
    LoadImage(&rect, tim.paddr);
    FreeHeapBlock(image);
    gMenuFrame = 0;
}

void FreeMenuQuads(void)
{
    FreeHeapBlock(gMenuQuads);
    gMenuFrame = 0;
}

void FreeTileGrid(void)
{
    FreeHeapBlock(gMenuQuads);
    gMenuFrame = 0;
}

void SortSpriteQuad(SpriteQuad *entry, GsOT *ot, u_short index)
{
    setPolyFT4(&entry->poly);
    setRGB0(&entry->poly, entry->r, entry->g, entry->b);
    entry->poly.tpage = entry->tpage;
    entry->poly.clut = entry->clut;
    setSemiTrans(&entry->poly, entry->attribute & 0x40000000);
    if (entry->attribute & 0x800000) {
        entry->poly.u0 = entry->u + entry->w - 1;
        entry->poly.u1 = entry->u;
        entry->poly.u2 = entry->u + entry->w - 1;
        entry->poly.u3 = entry->u;
    } else {
        entry->poly.u0 = entry->u;
        entry->poly.u1 = entry->u + entry->w - 1;
        entry->poly.u2 = entry->u;
        entry->poly.u3 = entry->u + entry->w - 1;
    }
    if (entry->attribute & 0x400000) {
        entry->poly.v0 = entry->v + entry->h - 1;
        entry->poly.v1 = entry->v + entry->h - 1;
        entry->poly.v2 = entry->v;
        entry->poly.v3 = entry->v;
    } else {
        entry->poly.v0 = entry->v;
        entry->poly.v1 = entry->v;
        entry->poly.v2 = entry->v + entry->h - 1;
        entry->poly.v3 = entry->v + entry->h - 1;
    }
    entry->poly.x0 = entry->x;
    entry->poly.y0 = entry->y;
    entry->poly.x1 = entry->x + entry->w;
    entry->poly.y1 = entry->y;
    entry->poly.x2 = entry->x;
    entry->poly.y2 = entry->y + entry->h;
    entry->poly.x3 = entry->x + entry->w;
    entry->poly.y3 = entry->y + entry->h;
    GsSortPoly(&entry->poly, ot, index);
}

void SortRotatedSpriteQuad(SpriteQuad *entry, GsOT *ot, u_short index)
{
    VECTOR corners[4];
    VECTOR rotated[4];
    SVECTOR angles;
    MATRIX rotation;
    int halfWidth, halfHeight;
    int centerX, centerY;
    int depth, zOffset;
    int i;

    setPolyFT4(&entry->poly);
    setRGB0(&entry->poly, entry->r, entry->g, entry->b);
    entry->poly.tpage = entry->tpage;
    entry->poly.clut = entry->clut;
    setSemiTrans(&entry->poly, entry->attribute & 0x40000000);
    if (entry->attribute & 0x800000) {
        entry->poly.u0 = entry->u + entry->w - 1;
        entry->poly.u1 = entry->u;
        entry->poly.u2 = entry->u + entry->w - 1;
        entry->poly.u3 = entry->u;
    } else {
        entry->poly.u0 = entry->u;
        entry->poly.u1 = entry->u + entry->w - 1;
        entry->poly.u2 = entry->u;
        entry->poly.u3 = entry->u + entry->w - 1;
    }
    if (entry->attribute & 0x400000) {
        entry->poly.v0 = entry->v + entry->h - 1;
        entry->poly.v1 = entry->v + entry->h - 1;
        entry->poly.v2 = entry->v;
        entry->poly.v3 = entry->v;
    } else {
        entry->poly.v0 = entry->v;
        entry->poly.v1 = entry->v;
        entry->poly.v2 = entry->v + entry->h - 1;
        entry->poly.v3 = entry->v + entry->h - 1;
    }
    halfWidth = entry->w / 2;
    halfHeight = entry->h / 2;
    centerX = entry->x + halfWidth;
    centerY = entry->y + halfHeight;
    PushMatrix();
    angles.vx = entry->rotateX;
    angles.vy = entry->rotateY;
    angles.vz = entry->rotateZ;
    RotMatrix_gte(&angles, &rotation);
    SetRotMatrix(&rotation);

    corners[0].vx = -halfWidth * entry->scale[0] / 4096;
    corners[0].vy = -halfHeight * entry->scale[1] / 4096;
    corners[0].vz = 0;
    corners[1].vx = halfWidth * entry->scale[0] / 4096;
    corners[1].vy = -halfHeight * entry->scale[1] / 4096;
    corners[1].vz = 0;
    corners[2].vx = -halfWidth * entry->scale[0] / 4096;
    corners[2].vy = halfHeight * entry->scale[1] / 4096;
    corners[2].vz = 0;
    corners[3].vx = halfWidth * entry->scale[0] / 4096;
    corners[3].vy = halfHeight * entry->scale[1] / 4096;
    corners[3].vz = 0;
    ApplyRotMatrixLV(&corners[0], &rotated[0]);
    ApplyRotMatrixLV(&corners[1], &rotated[1]);
    ApplyRotMatrixLV(&corners[2], &rotated[2]);
    ApplyRotMatrixLV(&corners[3], &rotated[3]);

    {
        int minimum = rotated[0].vz;
        for (i = 1; i < 4; i++) {
            if (rotated[i].vz < minimum) {
                minimum = rotated[i].vz;
            }
        }
        if (minimum < 0) {
            minimum = -minimum;
        }
        minimum += 350;
        depth = minimum;
    }
    zOffset = depth;
    for (i = 0; i < 4; i++) {
        rotated[i].vz += zOffset;
    }
    entry->poly.x0 = depth * rotated[0].vx / rotated[0].vz + centerX;
    entry->poly.y0 = depth * rotated[0].vy / rotated[0].vz + centerY;
    entry->poly.x1 = depth * rotated[1].vx / rotated[1].vz + centerX;
    entry->poly.y1 = depth * rotated[1].vy / rotated[1].vz + centerY;
    entry->poly.x2 = depth * rotated[2].vx / rotated[2].vz + centerX;
    entry->poly.y2 = depth * rotated[2].vy / rotated[2].vz + centerY;
    entry->poly.x3 = depth * rotated[3].vx / rotated[3].vz + centerX;
    entry->poly.y3 = depth * rotated[3].vy / rotated[3].vz + centerY;
    PopMatrix();
    GsSortPoly(&entry->poly, ot, index);
}

u_char gTileNumbers[52] = {
    1, 2, 3, 4, 3, 2, 1, 5, 6, 7, 8, 7, 6, 5, 9, 10, 11, 12, 11, 10, 9, 13, 14, 15,
    16, 15, 14, 13, 9, 10, 11, 12, 11, 10, 9, 5, 6, 7, 8, 7, 6, 5, 1, 2, 3, 4, 3, 2,
    1, 0, 0, 0,
};
u_char gTileMirror[50][2] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 1 },
    { 0, 1 },
    { 0, 1 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 1 },
    { 0, 1 },
    { 0, 1 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 1 },
    { 0, 1 },
    { 0, 1 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 1 },
    { 0, 1 },
    { 0, 1 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 1 },
    { 1, 1 },
    { 1, 1 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 1 },
    { 1, 1 },
    { 1, 1 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1, 1 },
    { 1, 1 },
    { 1, 1 },
    { 0, 0 },
};
MessageImage gMessageImages[17] = {
    { 0, 0, 112, 16, 1 },
    { 0, 8, 160, 16, 1 },
    { 0, 16, 240, 48, 3 },
    { 0, 64, 80, 8, 1 },
    { 0, 64, 136, 8, 1 },
    { 0, 72, 248, 24, 3 },
    { 0, 96, 224, 56, 4 },
    { 0, 152, 152, 16, 1 },
    { 0, 168, 248, 16, 1 },
    { 0, 184, 56, 8, 1 },
    { 56, 184, 120, 8, 1 },
    { 0, 192, 88, 8, 1 },
    { 0, 200, 104, 8, 1 },
    { 104, 200, 80, 8, 1 },
    { 0, 208, 56, 16, 1 },
    { 56, 208, 120, 8, 1 },
    { 0, 16, 240, 16, 1 },
};
MenuItem gMenuItems[5] = {
    { 8, 4, 1, 3 },
    { 19, 5, 1, 2 },
    { 24, 6, 1, 1 },
    { 36, 4, 0, 2 },
    { 47, 4, 0, 1 },
};
u_char gTileFlipDelay[49] = {
    6, 7, 8, 9, 10, 11, 12,
    5, 6, 7, 8, 9, 10, 11,
    4, 5, 6, 7, 8, 9, 10,
    3, 4, 5, 6, 7, 8, 9,
    2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6,
};
