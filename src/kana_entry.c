/* The start screen, which loads a save from the memory card or opens the kana name entry, and the
   name entry, which spells the chosen kana in romaji into gPlayerName. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <libmcrd.h>
#include <rand.h>

#include "sprite_quad.h"
#include "loading.h"
#include "graphics.h"
#include "grid_menu.h"
#include "heap.h"
#include "kana_entry.h"
#include "line_effect.h"
#include "memcard.h"
#include "pad.h"
#include "sound.h"
#include "system_menu.h"
#include "text.h"

extern u_char gRomajiVowels[];
extern u_char gRomajiConsonants[];
extern u_char gRomajiYVowels[];
extern u_char gRomajiVoicedConsonants[];

u_long *gStripTim = 0;
u_int gStripScrollStep;

void ScrollStripQuads(POLY_FT4 *left, POLY_FT4 *right, u_char shade)
{
    RECT rect;
    TIM_IMAGE tim;
    int rows;

    setPolyFT4(left);
    setPolyFT4(right);
    setRGB0(left, shade, shade, shade);
    setRGB0(right, shade, shade, shade);
    setXY4(left, 58, -23, 181, -7, 21, 254, 144, 270);
    setUV4(left, 0, 0, 128, 0, 0, 255, 128, 255);
    setXY4(right, -23, -1, 37, -22, 243, 353, 303, 332);
    setUV4(right, 0, 0, 128, 0, 0, 255, 128, 255);
    left->tpage = 23;
    right->tpage = 23;
    left->clut = 0x3d14;
    right->clut = 0x3d14;
    setSemiTrans(left, 1);
    setSemiTrans(right, 1);
    if (gStripScrollStep <= 128) {
        rect.x = 448;
        rect.y = 256;
        rect.h = 256;
        rect.w = 64;
        ClearImage(&rect, 0, 0, 0);
        rows = gStripScrollStep * 2;
        if (rows != 0) {
            OpenTIM(gStripTim);
            ReadTIM(&tim);
            rect.w = 32;
            rect.h = rows;
            rect.y = 512 - gStripScrollStep * 2;
            LoadImage(&rect, tim.paddr);
        }
    } else {
        OpenTIM(gStripTim);
        ReadTIM(&tim);
        rect.x = 448;
        rect.y = 256;
        rect.w = 32;
        rows = (gStripScrollStep & 0x7f) * 2;
        rect.h = 256 - rows;
        LoadImage(&rect, tim.paddr + (gStripScrollStep & 0x7f) * 32);
        if (rows != 0) {
            rect.y = 512 - rows;
            rect.h = rows;
            LoadImage(&rect, tim.paddr);
        }
    }
    gStripScrollStep++;
}

enum StartScreenState {
    START_OPENING,
    START_SHOW_CHOICES,
    START_CHOOSE,
    START_LOAD_OPEN,
    START_LOAD_CONFIRM,
    START_LOAD,
    START_LOAD_CLOSE = 9,
    START_NAME_ENTRY_OPEN,
    START_NAME_ENTRY,
    START_NAME_ENTRY_CLOSE
};

enum OpeningFlash {
    OPENING_FLASH_BASE,
    OPENING_FLASH_SEQ_A,
    OPENING_FLASH_SEQ_B
};

int RunStartScreen(void)
{
    short pos[2];
    short wait[2][6];
    u_char seq[2][6] = { { 3, 4, 5, 6, 7, 8 }, { 9, 10, 11, 6, 12, 8 } };
    RECT rect;
    GsSPRITE sprites[21];
    SpriteQuad quads[8];
    POLY_FT4 polyA;
    POLY_FT4 polyB;
    int cur;
    int choice;
    int choosing;
    u_long *image;
    CardSaveBlock *buf;
    int state;
    int i;
    int j;
    int sub;
    int waitCount;
    int ev;
    int r;
    int n;
    SpriteDesc *d;

    gStripScrollStep = 0;
    image = (u_long *)AllocHeapBlock(0x4000);
    rect.x = 320;
    rect.y = 480;
    rect.w = 256;
    rect.h = 32;
    StoreImage2(&rect, image);
    while (StepTimLoadSequence(0) == 0) {
    }
    for (i = 0; i < 14; i++) {
        d = &gStartScreenSprites[i];
        sprites[i].attribute = 0;
        sprites[i].x = d->x;
        sprites[i].y = d->y;
        sprites[i].w = d->w;
        sprites[i].h = d->h;
        sprites[i].tpage = d->tpage;
        sprites[i].u = d->u;
        sprites[i].v = d->v;
        sprites[i].cx = d->cx;
        sprites[i].cy = d->cy;
        sprites[i].r = 128;
        sprites[i].g = 128;
        sprites[i].b = 128;
        sprites[i].scalex = ONE;
        sprites[i].scaley = ONE;
        sprites[i].rotate = 0;
    }
    for (i = 18; i < 22; i++) {
        d = &gStartScreenSprites[i];
        sprites[i - 4].attribute = 0;
        sprites[i - 4].x = d->x;
        sprites[i - 4].y = d->y;
        sprites[i - 4].w = d->w;
        sprites[i - 4].h = d->h;
        sprites[i - 4].tpage = d->tpage;
        sprites[i - 4].u = d->u;
        sprites[i - 4].v = d->v;
        sprites[i - 4].cx = d->cx;
        sprites[i - 4].cy = d->cy;
        sprites[i - 4].r = 128;
        sprites[i - 4].g = 128;
        sprites[i - 4].b = 128;
        sprites[i - 4].scalex = ONE;
        sprites[i - 4].scaley = ONE;
        sprites[i - 4].rotate = 0;
    }
    for (i = 26; i < 29; i++) {
        d = &gStartScreenSprites[i];
        sprites[i - 8].attribute = 0;
        sprites[i - 8].x = d->x;
        sprites[i - 8].y = d->y;
        sprites[i - 8].w = d->w;
        sprites[i - 8].h = d->h;
        sprites[i - 8].tpage = d->tpage;
        sprites[i - 8].u = d->u;
        sprites[i - 8].v = d->v;
        sprites[i - 8].cx = d->cx;
        sprites[i - 8].cy = d->cy;
        sprites[i - 8].r = 128;
        sprites[i - 8].g = 128;
        sprites[i - 8].b = 128;
        sprites[i - 8].scalex = ONE;
        sprites[i - 8].scaley = ONE;
        sprites[i - 8].rotate = 0;
    }
    for (i = 0; i < 8; i++) {
        u_short cy, cx;
        if (i < 4) {
            ev = i + 14;
        } else {
            ev = i + 18;
        }
        d = &gStartScreenSprites[ev];
        quads[i].attribute = 0;
        quads[i].x = d->x;
        quads[i].y = d->y;
        quads[i].w = d->w;
        quads[i].h = d->h;
        quads[i].tpage = d->tpage;
        quads[i].u = d->u;
        quads[i].v = d->v;
        cy = d->cy;
        cx = d->cx;
        quads[i].r = 128;
        quads[i].g = 128;
        quads[i].b = 128;
        quads[i].scale[0] = ONE;
        quads[i].scale[1] = ONE;
        quads[i].rotateX = 0;
        quads[i].rotateY = 0;
        quads[i].rotateZ = 0;
        quads[i].clut = getClut(cx, cy);
    }
    quads[1].attribute = 0x800000;
    quads[4].attribute = 0x800000;
    quads[5].attribute = 0x40000000;
    quads[6].attribute = 0x40800000;
    UploadTextToVram("Are you sure?", 704, 368, TEXT_UPLOAD_LARGE, 0);
    UploadTextToVram("Yes", 704, 392, TEXT_UPLOAD_LARGE, 0);
    UploadTextToVram("No", 704, 416, TEXT_UPLOAD_LARGE, 0);
    for (i = 0; i < 6; i++) {
        wait[0][i] = rand() % 40 + 10;
        wait[1][i] = rand() % 40 + 10;
    }
    cur = 0;
    pos[0] = 0;
    pos[1] = 0;
    InitLineTrails();
    SetDispMask(1);
    state = START_OPENING;
    i = 0;
    for (;;) {
        DrawSync(0);
        BeginFrame();
        ReadPadEvent();
        switch (state) {
        case START_OPENING:
            StepLineTrails();
            for (j = 0; j < 2; j++) {
                GsSortFastSprite(&sprites[j], &gSubOt[gActiveBuffer], 1);
            }
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            wait[0][pos[0]]--;
            wait[1][pos[1]]--;
            ev = OPENING_FLASH_BASE;
            if (wait[0][pos[0]] == 0) {
                pos[0]++;
                ev = OPENING_FLASH_SEQ_A;
            }
            if (wait[1][pos[1]] == 0) {
                pos[1]++;
                if (ev == OPENING_FLASH_BASE) {
                    ev = OPENING_FLASH_SEQ_B;
                }
            }
            switch (ev) {
            case OPENING_FLASH_BASE:
                cur = seq[0][0];
                break;
            case OPENING_FLASH_SEQ_A:
                cur = seq[0][pos[0]];
                break;
            case OPENING_FLASH_SEQ_B:
                cur = seq[1][pos[1]];
                break;
            }
            GsSortFastSprite(&sprites[cur], &gSubOt[gActiveBuffer], 1);
            if (rand() & 1) {
                sprites[2].y = 197;
            } else {
                sprites[2].y = 189;
            }
            GsSortFastSprite(&sprites[2], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[0], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[1], &gSubOt[gActiveBuffer], 1);
            n = i % 16;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            SortRotatedSpriteQuad(&quads[2], &gSubOt[gActiveBuffer], 1);
            i++;
            if (pos[0] >= 5 || pos[1] >= 5) {
                choice = 0;
                i = 0;
                state++;
            }
            break;
        case START_SHOW_CHOICES:
            sprites[0].r = 128 - i * 4;
            sprites[0].g = 128 - i * 4;
            sprites[0].b = 128 - i * 4;
            sprites[1].r = 128 - i * 4;
            sprites[1].g = 128 - i * 4;
            sprites[1].b = 128 - i * 4;
            for (j = 0; j < 2; j++) {
                GsSortFastSprite(&sprites[j], &gSubOt[gActiveBuffer], 1);
            }
            sprites[13].x = i * 105 / 32 + 111;
            sprites[13].y = i * -78 / 32 + 100;
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            quads[0].r = 128 - i * 4;
            quads[0].g = 128 - i * 4;
            quads[0].b = 128 - i * 4;
            quads[1].r = 128 - i * 4;
            quads[1].g = 128 - i * 4;
            quads[1].b = 128 - i * 4;
            SortSpriteQuad(&quads[0], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[1], &gSubOt[gActiveBuffer], 1);
            n = i % 16;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            quads[3].x = 152;
            quads[3].y = i * -54 / 32 + 172;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            quads[2].r = 128 - i * 4;
            quads[2].g = 128 - i * 4;
            quads[2].b = 128 - i * 4;
            SortRotatedSpriteQuad(&quads[2], &gSubOt[gActiveBuffer], 1);
            if (choice == 0) {
                sprites[14].cx = 352;
                sprites[15].cx = 336;
            } else {
                sprites[14].cx = 336;
                sprites[15].cx = 352;
            }
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            if (choice != 0) {
                GsSortFastSprite(&sprites[17], &gSubOt[gActiveBuffer], 1);
            } else {
                GsSortFastSprite(&sprites[16], &gSubOt[gActiveBuffer], 1);
            }
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            i++;
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            if (i >= 33) {
                choosing = 1;
                i = 0;
                state++;
            }
            break;
        case START_CHOOSE:
            switch (gPadEvent) {
            case PAD_EVENT_UP:
            case PAD_EVENT_UP_LEFT:
            case PAD_EVENT_UP_RIGHT:
                PlaySound(1);
                choice = 0;
                break;
            case PAD_EVENT_DOWN:
            case PAD_EVENT_DOWN_LEFT:
            case PAD_EVENT_DOWN_RIGHT:
                PlaySound(1);
                choice = 1;
                break;
            case PAD_EVENT_CIRCLE:
                PlaySound(0);
                choosing = 0;
                break;
            }
            DrawSync(0);
            BeginFrame();
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            n = i % 64 / 4;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            if (choice == 0) {
                sprites[14].cx = 352;
                sprites[15].cx = 336;
            } else {
                sprites[14].cx = 336;
                sprites[15].cx = 352;
            }
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            if (choice != 0) {
                GsSortFastSprite(&sprites[17], &gSubOt[gActiveBuffer], 1);
            } else {
                GsSortFastSprite(&sprites[16], &gSubOt[gActiveBuffer], 1);
            }
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            i++;
            i %= 64;
            if (choosing == 0) {
                j = 0;
                if (choice != 0) {
                    state++;
                } else {
                    state = START_NAME_ENTRY_OPEN;
                }
            }
            break;
        case START_LOAD_OPEN:
            n = 128 - j * 4;
            sprites[13].r = n;
            sprites[13].g = n;
            sprites[13].b = n;
            sprites[14].r = n;
            sprites[14].g = n;
            sprites[14].b = n;
            quads[3].r = n;
            quads[3].g = n;
            quads[3].b = n;
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            n = i % 64 / 4;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            sprites[15].x = j * -91 / 32 + 106;
            sprites[15].y = j + 157;
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            i++;
            i %= 64;
            j++;
            if (j >= 33) {
                choice = 0;
                choosing = 1;
                state++;
            }
            break;
        case START_LOAD_CONFIRM:
            switch (gPadEvent) {
            case PAD_EVENT_UP_LEFT:
            case PAD_EVENT_LEFT:
            case PAD_EVENT_DOWN_LEFT:
                PlaySound(1);
                choice = 0;
                break;
            case PAD_EVENT_UP_RIGHT:
            case PAD_EVENT_RIGHT:
            case PAD_EVENT_DOWN_RIGHT:
                PlaySound(1);
                choice = 1;
                break;
            case PAD_EVENT_CIRCLE:
                if (choice == 0) {
                    PlaySound(26);
                } else {
                    PlaySound(27);
                }
                choosing = 0;
                break;
            }
            quads[7].x = choice ? 230 : 34;
            SortSpriteQuad(&quads[4], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[5], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[6], &gSubOt[gActiveBuffer], 1);
            SortSpriteQuad(&quads[7], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[18], &gSubOt[gActiveBuffer], 0);
            GsSortFastSprite(&sprites[19], &gSubOt[gActiveBuffer], 0);
            GsSortFastSprite(&sprites[20], &gSubOt[gActiveBuffer], 0);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            if (choosing == 0) {
                j = 0;
                if (choice == 0) {
                    sub = MEMCARD_LOAD_ACCEPT;
                    state++;
                } else {
                    state = START_LOAD_CLOSE;
                }
            }
            break;
        case START_LOAD:
            switch (sub) {
            case MEMCARD_LOAD_ACCEPT:
                AcceptMemCard();
                sub++;
                break;
            case MEMCARD_LOAD_POLL_ACCEPT:
                r = PollMemCardSync();
                if (r != 1) {
                    r = -r;
                    switch (r) {
                    case McErrCardNotExist:
                        PlaySound(28);
                        sub = MEMCARD_LOAD_ERR_NO_CARD;
                        waitCount = 60;
                        break;
                    case McErrCardInvalid:
                        PlaySound(28);
                        sub = MEMCARD_LOAD_ERR_INVALID;
                        waitCount = 60;
                        break;
                    case McErrNewCard:
                        sub = MEMCARD_LOAD_ACCEPT;
                        break;
                    case McErrNotFormat:
                        PlaySound(28);
                        sub = MEMCARD_LOAD_ERR_UNFORMATTED;
                        waitCount = 60;
                        break;
                    }
                } else {
                    sub = MEMCARD_LOAD_OPEN;
                }
                break;
            case MEMCARD_LOAD_OPEN:
                switch (-OpenMemCardFile(0)) {
                case McErrNone:
                    sub++;
                    waitCount = 10;
                    break;
                case McErrCardNotExist:
                    PlaySound(28);
                    sub = MEMCARD_LOAD_ERR_NO_CARD;
                    waitCount = 60;
                    break;
                case McErrCardInvalid:
                    PlaySound(28);
                    sub = MEMCARD_LOAD_ERR_INVALID;
                    waitCount = 60;
                    break;
                case McErrNotFormat:
                    PlaySound(28);
                    sub = MEMCARD_LOAD_ERR_UNFORMATTED;
                    waitCount = 60;
                    break;
                case McErrFileNotExist:
                    PlaySound(28);
                    sub = MEMCARD_LOAD_ERR_NO_FILE;
                    waitCount = 60;
                    break;
                }
                break;
            case MEMCARD_LOAD_START_READ:
                buf = AllocHeapBlock(0x6000);
                ReadMemCardFile(buf, PackCardSaveBlock(buf) << 7);
                DrawMessageWindow(MESSAGE_LOADING);
                sub = MEMCARD_LOAD_POLL_READ;
                break;
            case MEMCARD_LOAD_POLL_READ:
                DrawMessageWindow(MESSAGE_LOADING);
                r = PollMemCardSync();
                if (r != 0) {
                    if (r == 1) {
                        if (UnpackCardSaveBlock(buf) != 0) {
                            sub++;
                        } else {
                            sub += 2;
                        }
                        waitCount = 30;
                        CloseMemCardFile();
                    } else {
                        r = -r;
                        switch (r) {
                        case McErrCardNotExist:
                            sub = MEMCARD_LOAD_ACCEPT;
                            if (--waitCount == 0) {
                                PlaySound(28);
                                sub = MEMCARD_LOAD_ERR_NO_CARD;
                                waitCount = 60;
                                CloseMemCardFile();
                            }
                            CloseMemCardFile();
                            FreeHeapBlock(buf);
                            break;
                        case McErrCardInvalid:
                            PlaySound(28);
                            sub = MEMCARD_LOAD_ERR_INVALID;
                            waitCount = 60;
                            FreeHeapBlock(buf);
                            CloseMemCardFile();
                            break;
                        case McErrNewCard:
                            sub = MEMCARD_LOAD_ACCEPT;
                            FreeHeapBlock(buf);
                            CloseMemCardFile();
                            break;
                        }
                    }
                }
                break;
            case MEMCARD_LOAD_DONE:
                if (--waitCount == 0) {
                    RestoreGridCursor();
                    FreeHeapBlock(buf);
                    goto exit;
                }
                DrawMessageWindow(MESSAGE_LOADED);
                break;
            case MEMCARD_LOAD_ERR_CORRUPT:
                if (--waitCount == 0) {
                    FreeHeapBlock(buf);
                    state = START_LOAD_CLOSE;
                }
                DrawMessageWindow(MESSAGE_SAVE_CORRUPT);
                break;
            case MEMCARD_LOAD_ERR_NO_CARD:
                if (--waitCount == 0) {
                    state = START_LOAD_CLOSE;
                }
                DrawMessageWindow(MESSAGE_NO_CARD);
                break;
            case MEMCARD_LOAD_ERR_INVALID:
                if (--waitCount == 0) {
                    state = START_LOAD_CLOSE;
                }
                DrawMessageWindow(MESSAGE_INVALID_CARD);
                break;
            case MEMCARD_LOAD_ERR_UNFORMATTED:
                if (--waitCount == 0) {
                    state = START_LOAD_CLOSE;
                }
                DrawMessageWindow(MESSAGE_UNFORMATTED);
                break;
            case MEMCARD_LOAD_ERR_NO_FILE:
                if (--waitCount == 0) {
                    state = START_LOAD_CLOSE;
                }
                DrawMessageWindow(MESSAGE_NO_SAVE);
                break;
            }
            SortSpriteQuad(&quads[4], &gSubOt[gActiveBuffer], 2);
            SortSpriteQuad(&quads[5], &gSubOt[gActiveBuffer], 2);
            SortSpriteQuad(&quads[6], &gSubOt[gActiveBuffer], 2);
            SortSpriteQuad(&quads[7], &gSubOt[gActiveBuffer], 2);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 2);
            GsSortFastSprite(&sprites[18], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[19], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[20], &gSubOt[gActiveBuffer], 1);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            break;
        case START_LOAD_CLOSE:
            n = j * 4;
            sprites[13].r = n;
            sprites[13].g = n;
            sprites[13].b = n;
            sprites[14].r = n;
            sprites[14].g = n;
            sprites[14].b = n;
            quads[3].r = n;
            quads[3].g = n;
            quads[3].b = n;
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            n = i % 64 / 4;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            sprites[15].x = j * 91 / 32 + 15;
            sprites[15].y = j * -32 / 32 + 189;
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            i++;
            i %= 64;
            j++;
            if (j >= 33) {
                choice = 1;
                choosing = 1;
                state = START_CHOOSE;
            }
            break;
        case START_NAME_ENTRY_OPEN:
            n = 128 - j * 4;
            sprites[13].r = n;
            sprites[13].g = n;
            sprites[13].b = n;
            sprites[15].r = n;
            sprites[15].g = n;
            sprites[15].b = n;
            quads[3].r = n;
            quads[3].g = n;
            quads[3].b = n;
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            n = i % 64 / 4;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            sprites[14].x = j * 71 / 32 + 83;
            sprites[14].y = j * -54 / 32 + 73;
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            i++;
            i %= 64;
            j++;
            if (j >= 33) {
                choice = 0;
                choosing = 1;
                state++;
            }
            break;
        case START_NAME_ENTRY:
            if (RunNameEntry() != 0) {
                goto exit;
            }
            j = 0;
            state++;
            break;
        case START_NAME_ENTRY_CLOSE:
            n = j * 4;
            sprites[13].r = n;
            sprites[13].g = n;
            sprites[13].b = n;
            sprites[15].r = n;
            sprites[15].g = n;
            sprites[15].b = n;
            quads[3].r = n;
            quads[3].g = n;
            quads[3].b = n;
            GsSortFastSprite(&sprites[13], &gSubOt[gActiveBuffer], 1);
            n = i % 64 / 4;
            quads[3].rotateZ = (n * 256 + 256) % 4096;
            SortRotatedSpriteQuad(&quads[3], &gSubOt[gActiveBuffer], 1);
            sprites[14].x = j * -71 / 32 + 154;
            sprites[14].y = j * 54 / 32 + 19;
            GsSortFastSprite(&sprites[14], &gSubOt[gActiveBuffer], 1);
            GsSortFastSprite(&sprites[15], &gSubOt[gActiveBuffer], 1);
            ScrollStripQuads(&polyA, &polyB, 64);
            GsSortPoly(&polyA, &gSubOt[gActiveBuffer], 10);
            GsSortPoly(&polyB, &gSubOt[gActiveBuffer], 10);
            i++;
            i %= 64;
            j++;
            if (j >= 33) {
                state = START_CHOOSE;
                choice = 0;
                choosing = 1;
            }
            break;
        }
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
    }
exit:
    DrawSync(0);
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    rect.x = 0;
    rect.y = 0;
    rect.w = 640;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    FreeLineTrails();
    rect.x = 320;
    rect.y = 480;
    rect.w = 256;
    rect.h = 32;
    LoadImage2(&rect, image);
    FreeHeapBlock(gStripTim);
    FreeHeapBlock(image);
}

int InitKanaEntry(KanaEntry *menu)
{
    int i, col;

    for (i = 0; i < 9; i++) {
        for (col = 0; col < 9; col++) {
            setPolyFT4(&menu->cells[i * 9 + col]);
            menu->cells[i * 9 + col].clut = 0x3c54;
            menu->cells[i * 9 + col].r0 = gKanaCellShades[i * 9 + col];
            menu->cells[i * 9 + col].g0 = gKanaCellShades[i * 9 + col];
            menu->cells[i * 9 + col].b0 = gKanaCellShades[i * 9 + col];
            menu->cells[i * 9 + col].x0 = gKanaGridX[i + 1][col];
            menu->cells[i * 9 + col].y0 = gKanaGridY[i + 1][col];
            menu->cells[i * 9 + col].x1 = gKanaGridX[i][col];
            menu->cells[i * 9 + col].y1 = gKanaGridY[i][col];
            menu->cells[i * 9 + col].x2 = gKanaGridX[i + 1][col + 1];
            menu->cells[i * 9 + col].y2 = gKanaGridY[i + 1][col + 1];
            menu->cells[i * 9 + col].x3 = gKanaGridX[i][col + 1];
            menu->cells[i * 9 + col].y3 = gKanaGridY[i][col + 1];
        }
    }

    menu->highlight.clut = 0x3c55;
    setPolyFT4(&menu->highlight);
    setRGB0(&menu->highlight, 128, 128, 128);
    menu->highlight.x0 = 0xa7;
    menu->highlight.y0 = 0x66;
    menu->highlight.x1 = 0xec;
    menu->highlight.y1 = 0x74;
    menu->highlight.x2 = 0x92;
    menu->highlight.y2 = 0x7e;
    menu->highlight.x3 = 0xdb;
    menu->highlight.y3 = 0x8f;

    menu->sprites[0].attribute = 0;
    menu->sprites[0].x = 0x9a;
    menu->sprites[0].y = 0x13;
    menu->sprites[0].w = 0xa8;
    menu->sprites[0].h = 0x10;
    menu->sprites[0].u = 0;
    menu->sprites[0].v = 0x18;
    menu->sprites[0].tpage = 0x16;
    menu->sprites[0].cx = 0x160;
    menu->sprites[0].cy = 0xf2;
    menu->sprites[0].r = 128;
    menu->sprites[0].g = 128;
    menu->sprites[0].b = 128;

    menu->sprites[1].attribute = 0;
    menu->sprites[1].x = 0x92;
    menu->sprites[1].y = 0x2a;
    menu->sprites[1].w = 0xa8;
    menu->sprites[1].h = 0x10;
    menu->sprites[1].u = 0;
    menu->sprites[1].v = 0x28;
    menu->sprites[1].tpage = 0x16;
    menu->sprites[1].cx = 0x160;
    menu->sprites[1].cy = 0xf2;
    menu->sprites[1].r = 128;
    menu->sprites[1].g = 128;
    menu->sprites[1].b = 128;

    menu->sprites[2].attribute = 0;
    menu->sprites[2].x = 0x40;
    menu->sprites[2].y = 0x20;
    menu->sprites[2].w = 0x100;
    menu->sprites[2].h = 8;
    menu->sprites[2].u = 0;
    menu->sprites[2].v = 0;
    menu->sprites[2].tpage = 0x16;
    menu->sprites[2].cx = 0x140;
    menu->sprites[2].cy = 0xf2;
    menu->sprites[2].r = 128;
    menu->sprites[2].g = 128;
    menu->sprites[2].b = 128;

    menu->sprites[3].attribute = 0;
    menu->sprites[3].x = 0;
    menu->sprites[3].y = 0x97;
    menu->sprites[3].w = 0x98;
    menu->sprites[3].h = 8;
    menu->sprites[3].u = 0;
    menu->sprites[3].v = 0x38;
    menu->sprites[3].tpage = 0x16;
    menu->sprites[3].cx = 0x140;
    menu->sprites[3].cy = 0xf2;
    menu->sprites[3].r = 128;
    menu->sprites[3].g = 128;
    menu->sprites[3].b = 128;

    menu->sprites[4].attribute = 0;
    menu->sprites[4].x = 0x98;
    menu->sprites[4].y = 0x67;
    menu->sprites[4].w = 0x48;
    menu->sprites[4].h = 0x38;
    menu->sprites[4].u = 0xa8;
    menu->sprites[4].v = 8;
    menu->sprites[4].tpage = 0x16;
    menu->sprites[4].cx = 0x140;
    menu->sprites[4].cy = 0xf2;
    menu->sprites[4].r = 128;
    menu->sprites[4].g = 128;
    menu->sprites[4].b = 128;

    for (i = 0; i < 8; i++) {
        menu->tiles[i].attribute = 0;
        menu->tiles[i].tpage = 21;
        menu->tiles[i].clut = 0x5015;
        menu->tiles[i].scale[0] = 4096;
        menu->tiles[i].scale[1] = 4096;
        menu->tiles[i].w = 16;
        menu->tiles[i].h = 16;
        menu->tiles[i].rotateX = 0;
        menu->tiles[i].rotateY = 0;
        menu->tiles[i].x = gKanaTilePositions[i][0];
        menu->tiles[i].y = gKanaTilePositions[i][1];
        menu->tiles[i].u = 0;
        menu->tiles[i].v = 8;
        menu->tiles[i].r = 128;
        menu->tiles[i].g = 128;
        menu->tiles[i].b = 128;
    }

    menu->tiles[0].rotateZ = 0x100;
    menu->tiles[1].rotateZ = 0x300;
    menu->tiles[2].rotateZ = 0x100;
    menu->tiles[3].rotateZ = 0x300;
    menu->tiles[4].rotateZ = 0x100;
    menu->tiles[5].rotateZ = 0x300;
    menu->tiles[6].rotateZ = 0x100;
    menu->tiles[7].rotateZ = 0x300;

    for (i = 0; i < 9; i++) {
        menu->glyphs[i].x = 136 - i * 16;
        menu->glyphs[i].attribute = 0;
        menu->glyphs[i].y = 135;
        menu->glyphs[i].w = 16;
        menu->glyphs[i].h = 16;
        menu->glyphs[i].tpage = 22;
        menu->glyphs[i].cx = 320;
        menu->glyphs[i].cy = 243;
        menu->glyphs[i].r = 128;
        menu->glyphs[i].g = 128;
        menu->glyphs[i].b = 128;
    }
    menu->row = 5;
    menu->col = 1;
    menu->count = 0;
}

void DrawKanaChart(KanaEntry *p)
{
    int row;
    int col;
    int y;
    int x;
    int u;
    int dy;
    int dx;
    int n;
    POLY_FT4 *cell;

    for (row = 0; row < 9; row++) {
        dy = row - 3;
        y = p->row + dy;
        if (y < 0) {
            continue;
        }
        if (y >= 13) {
            break;
        }
        for (col = 0; col < 9; col++) {
            dx = col - 4;
            x = p->col + dx;
            if ((unsigned int)x >= 5) {
                continue;
            }
            u = y;
            n = row * 9 + col;
            if (u < 5) {
                p->cells[n].tpage = 7;
            } else if (u < 10) {
                p->cells[n].tpage = 6;
                u -= 5;
            } else {
                p->cells[n].tpage = 5;
                u -= 10;
            }
            cell = &p->cells[n];
            setUV4(cell, (4 - u) * 48, x * 32, (4 - u) * 48 + 47, x * 32,
                   (4 - u) * 48, x * 32 + 31, (4 - u) * 48 + 47, x * 32 + 31);
            if (cell->x0 > 0 && cell->x1 > 0 && cell->x2 > 0 && cell->x3 > 0) {
                if (row == 3 && (col == 3 || col == 4)) {
                    if (col == 4) {
                        p->highlight.tpage = cell->tpage;
                        p->highlight.u0 = cell->u0;
                        p->highlight.v0 = cell->v0;
                        p->highlight.u1 = cell->u1;
                        p->highlight.v1 = cell->v1;
                        p->highlight.u2 = cell->u2;
                        p->highlight.v2 = cell->v2;
                        p->highlight.u3 = cell->u3;
                        p->highlight.v3 = cell->v3;
                        GsSortPoly(&p->highlight, &gSubOt[gActiveBuffer], 9);
                    }
                } else {
                    GsSortPoly(&p->cells[n], &gSubOt[gActiveBuffer], 10);
                }
            }
        }
    }
}

void DrawKanaText(KanaEntry *p)
{
    int i;
    int j;

    for (i = 0, j = p->count - 1; i < p->count; i++, j--) {
        if (p->count != 8) {
            p->glyphs[i + 1].u = p->text[j] % 15 * 16;
            p->glyphs[i + 1].v = p->text[j] / 15 * 16 + 65;
            GsSortFastSprite(&p->glyphs[i + 1], &gSubOt[gActiveBuffer], 2);
        } else {
            p->glyphs[i].u = p->text[j] % 15 * 16;
            p->glyphs[i].v = p->text[j] / 15 * 16 + 65;
            GsSortFastSprite(&p->glyphs[i], &gSubOt[gActiveBuffer], 2);
        }
    }
    if (p->count != 8) {
        p->glyphs[0].u = 0xe0;
        p->glyphs[0].v = 0x81;
        GsSortFastSprite(&p->glyphs[0], &gSubOt[gActiveBuffer], 2);
    }
}

void IncrementKanaRow(KanaEntry *p)
{
    short row = p->row;

    if (row < 12) {
        if (row == 6) {
            if (p->col != 1 && p->col != 3) {
                p->row = row + 1;
            } else {
                p->row += 2;
            }
        } else if (row == 8) {
            if (p->col == 0 || p->col == 4) {
                p->row = row + 1;
            } else {
                p->row = row + 2;
            }
        } else if (row == 10) {
            if (p->col != 1) {
                if (p->col == 3) {
                    p->row = row + 2;
                } else {
                    p->row = row + 1;
                }
            }
        } else {
            p->row = row + 1;
        }
    }
}

void ApplySelectedKana(KanaEntry *p)
{
    int accentColumn;
    u_char c;
    int d;
    u_char e;

    if (p->count < 8) {
        if (p->row < 10) {
            if (p->row == 9 && p->col == 4 && p->count == 0) {
                return;
            }
            if (p->row < 7) {
                p->text[p->count] = p->col + p->row * 5;
            } else if (p->row == 7) {
                p->text[p->count] = p->col / 2 + 35;
            } else if (p->row == 8) {
                p->text[p->count] = p->col + 38;
            } else {
                p->text[p->count] = p->col / 4 + 43;
            }
            p->count++;
            return;
        }
        accentColumn = 1;
        if (p->row == 10) {
            if (p->count == 0) {
                return;
            }
            switch (p->text[p->count - 1]) {
            case 62:
                if (p->col == accentColumn) {
                    p->text[p->count] = 76;
                    p->count++;
                }
                break;
            case 63:
                if (p->col == 2) {
                    p->text[p->count] = 77;
                    p->count++;
                }
                break;
            case 27:
                p->text[p->count] = p->col + 75;
                p->count++;
                break;
            }
            return;
        }
        if (p->row == 11) {
            if (p->count == 0) {
                return;
            }
            switch (p->text[p->count - 1]) {
            case 6:
            case 11:
            case 16:
            case 21:
            case 26:
            case 31:
            case 39:
            case 50:
            case 55:
            case 60:
            case 65:
            case 70:
                p->text[p->count] = p->col / 2 + 46;
                p->count++;
                break;
            }
            return;
        }
    }
    if (p->count == 0) {
        return;
    }
    if (p->row != 12) {
        return;
    }
    switch (p->col) {
    case 0:
        if (p->count < 8 && p->text[p->count - 1] != 45) {
            p->text[p->count] = 45;
            p->count++;
        }
        break;
    case 2:
        c = p->text[p->count - 1];
        if (c >= 5 && c < 20 && c != 16 && c != 17) {
            p->text[p->count - 1] = c + 44;
            return;
        }
        e = p->text[p->count - 1];
        if (e >= 25 && e < 30) {
            p->text[p->count - 1] = e + 39;
        }
        break;
    case 3:
        e = p->text[p->count - 1];
        if (e >= 25 && e < 30) {
            p->text[p->count - 1] = e + 44;
        }
        break;
    case 4:
        if (p->count < 8) {
            d = p->text[p->count - 1];
            if (d < 44 || (d >= 46 && d != 80)) {
                p->text[p->count] = 80;
                p->count++;
            }
        }
        break;
    }
}

int RunKanaEntryMenu(KanaEntry *menu)
{
    int i;

    for (;;) {
        DrawSync(0);
        BeginFrame();
        ReadPadEvent();
        switch (gPadEvent) {
        case PAD_EVENT_UP:
            if (menu->col > 0) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else if (menu->col == 4) {
                        menu->col = 2;
                    }
                } else if (menu->row == 9) {
                    menu->col = 0;
                } else if (menu->row == 12) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else {
                        menu->col--;
                    }
                } else {
                    menu->col--;
                }
            }
            break;
        case PAD_EVENT_UP_LEFT:
            IncrementKanaRow(menu);
            if (menu->col > 0) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else if (menu->col == 4) {
                        menu->col = 2;
                    }
                } else if (menu->row == 9) {
                    menu->col = 0;
                } else if (menu->row == 12) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else {
                        menu->col--;
                    }
                } else {
                    menu->col--;
                }
            }
            break;
        case PAD_EVENT_UP_RIGHT:
            if (menu->row > 0) {
                if (menu->row == 12) {
                    if (menu->col == 3) {
                        menu->row -= 2;
                    } else {
                        menu->row--;
                    }
                } else if (menu->row == 10) {
                    if (menu->col == 0 || menu->col == 4) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else if (menu->row == 8) {
                    if (menu->col != 1 && menu->col != 3) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else {
                    menu->row--;
                }
            }
            if (menu->col > 0) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else if (menu->col == 4) {
                        menu->col = 2;
                    }
                } else if (menu->row == 9) {
                    menu->col = 0;
                } else if (menu->row == 12) {
                    if (menu->col == 2) {
                        menu->col = 0;
                    } else {
                        menu->col--;
                    }
                } else {
                    menu->col--;
                }
            }
            break;
        case PAD_EVENT_LEFT:
            IncrementKanaRow(menu);
            break;
        case PAD_EVENT_RIGHT:
            if (menu->row > 0) {
                if (menu->row == 12) {
                    if (menu->col != 3) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else if (menu->row == 10) {
                    if (menu->col == 0 || menu->col == 4) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else if (menu->row == 8) {
                    if (menu->col != 1 && menu->col != 3) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else {
                    menu->row--;
                }
            }
            break;
        case PAD_EVENT_DOWN:
            if (menu->col < 4) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else if (menu->col == 2) {
                        menu->col = 4;
                    }
                } else if (menu->row == 9) {
                    if (menu->col == 0) {
                        menu->col = 4;
                    }
                } else if (menu->row == 12) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else {
                        menu->col++;
                    }
                } else {
                    menu->col++;
                }
            }
            break;
        case PAD_EVENT_DOWN_LEFT:
            IncrementKanaRow(menu);
            if (menu->col < 4) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else if (menu->col == 2) {
                        menu->col = 4;
                    }
                } else if (menu->row == 9) {
                    if (menu->col == 0) {
                        menu->col = 4;
                    }
                } else if (menu->row == 12) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else {
                        menu->col++;
                    }
                } else {
                    menu->col++;
                }
            }
            break;
        case PAD_EVENT_DOWN_RIGHT:
            if (menu->row > 0) {
                if (menu->row == 12) {
                    if (menu->col == 3) {
                        menu->row -= 2;
                    } else {
                        menu->row--;
                    }
                } else if (menu->row == 10) {
                    if (menu->col == 0 || menu->col == 4) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else if (menu->row == 8) {
                    if (menu->col != 1 && menu->col != 3) {
                        menu->row--;
                    } else {
                        menu->row -= 2;
                    }
                } else {
                    menu->row--;
                }
            }
            if (menu->col < 4) {
                if (menu->row == 7 || menu->row == 11) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else if (menu->col == 2) {
                        menu->col = 4;
                    }
                } else if (menu->row == 9) {
                    if (menu->col == 0) {
                        menu->col = 4;
                    }
                } else if (menu->row == 12) {
                    if (menu->col == 0) {
                        menu->col = 2;
                    } else {
                        menu->col++;
                    }
                } else {
                    menu->col++;
                }
            }
            break;
        case PAD_EVENT_CIRCLE:
            PlaySound(0);
            ApplySelectedKana(menu);
            break;
        case PAD_EVENT_CROSS:
            PlaySound(27);
            if (menu->count != 0) {
                if (menu->count > 0) {
                    menu->count--;
                }
            } else {
                return 0;
            }
            break;
        case PAD_EVENT_START:
            if (menu->count != 0) {
                PlaySound(30);
                return 1;
            }
            break;
        }
        DrawKanaChart(menu);
        for (i = 4; i >= 0; i--) {
            if (i != 1 || menu->count != 0) {
                GsSortFastSprite(&menu->sprites[i], &gSubOt[gActiveBuffer], 1);
            }
        }
        for (i = 0; i < 8; i++) {
            if (7 - i != menu->count) {
                SortRotatedSpriteQuad(&menu->tiles[i], &gSubOt[gActiveBuffer], 3);
            }
        }
        DrawKanaText(menu);
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
    }
}

int RomanizeKanaText(KanaEntry *entry)
{
    int i;
    int n;
    int c;

    n = 0;
    for (i = 0; i < entry->count; i++) {
        c = entry->text[i];
        if (c < 5) {
            gPlayerName[n++] = gRomajiVowels[c];
        } else if (c < 35) {
            if (c % 5 == 1 && i + 1 < entry->count && entry->text[i + 1] >= 46 && entry->text[i + 1] < 49) {
                gPlayerName[n++] = gRomajiConsonants[c / 5 - 1];
                gPlayerName[n++] = 'Y';
                gPlayerName[n++] = gRomajiYVowels[entry->text[i + 1] - 46];
                i++;
            } else {
                gPlayerName[n++] = gRomajiConsonants[c / 5 - 1];
                gPlayerName[n++] = gRomajiVowels[c % 5];
            }
        } else if (c < 38) {
            gPlayerName[n++] = 'Y';
            gPlayerName[n++] = gRomajiYVowels[c - 35];
        } else if (c < 43) {
            c -= 38;
            if (c == 1 && i + 1 < entry->count && entry->text[i + 1] >= 46 && entry->text[i + 1] < 49) {
                gPlayerName[n++] = 'R';
                gPlayerName[n++] = 'Y';
                gPlayerName[n++] = gRomajiYVowels[entry->text[i + 1] - 46];
                i++;
            } else {
                gPlayerName[n++] = 'R';
                gPlayerName[n++] = gRomajiVowels[c];
            }
        } else if (c == 43) {
            gPlayerName[n++] = 'W';
            gPlayerName[n++] = 'A';
        } else if (c == 44) {
            gPlayerName[n++] = 'N';
            gPlayerName[n++] = 'N';
        } else if (c == 45) {
            gPlayerName[n++] = '#';
        } else if (c >= 49 && c < 70) {
            c -= 49;
            if (c % 5 == 1 && i + 1 < entry->count && entry->text[i + 1] >= 46 && entry->text[i + 1] < 49) {
                gPlayerName[n++] = gRomajiVoicedConsonants[c / 5];
                gPlayerName[n++] = 'Y';
                gPlayerName[n++] = gRomajiYVowels[entry->text[i + 1] - 46];
                i++;
            } else {
                gPlayerName[n++] = gRomajiVoicedConsonants[c / 5];
                gPlayerName[n++] = gRomajiVowels[c % 5];
            }
        } else if (c >= 75 && c < 80) {
            c -= 75;
            switch (entry->text[i - 1]) {
            case 62:
                gPlayerName[n - 2] = 'D';
                gPlayerName[n - 1] = 'I';
                break;
            case 63:
                gPlayerName[n - 2] = 'D';
                gPlayerName[n - 1] = 'U';
                break;
            case 27:
                gPlayerName[n - 2] = 'F';
                gPlayerName[n - 1] = gRomajiVowels[c % 5];
                break;
            }
        } else if (c == 80) {
            gPlayerName[n++] = '-';
        }
    }
    gPlayerName[n] = 0;
}

int RunNameEntry(void)
{
    int result;
    int unused[2]; /* 8 bytes; nothing uses it */
    KanaEntry state;

    InitKanaEntry(&state);
    result = RunKanaEntryMenu(&state);
    RomanizeKanaText(&state);
    return result;
}

SpriteDesc gStartScreenSprites[29] = {
    { 16, 189, 112, 32, 21, 0, 0, 320, 408 },
    { 208, 189, 48, 16, 21, 208, 0, 320, 408 },
    { 192, 189, 16, 8, 21, 240, 16, 320, 408 },
    { 192, 205, 128, 16, 21, 0, 32, 320, 408 },
    { 192, 205, 128, 16, 21, 0, 48, 320, 408 },
    { 192, 205, 128, 16, 21, 0, 64, 320, 408 },
    { 192, 205, 128, 16, 21, 0, 80, 320, 408 },
    { 192, 205, 128, 8, 21, 0, 96, 320, 408 },
    { 192, 205, 128, 8, 21, 128, 80, 320, 408 },
    { 192, 205, 128, 16, 21, 128, 32, 320, 408 },
    { 192, 205, 128, 16, 21, 128, 48, 320, 408 },
    { 192, 205, 128, 16, 21, 128, 64, 320, 408 },
    { 192, 205, 128, 8, 21, 128, 96, 320, 408 },
    { 111, 100, 96, 32, 21, 112, 0, 336, 408 },
    { 0, 157, 160, 48, 21, 0, 104, 320, 408 },
    { 160, 157, 160, 48, 21, 0, 104, 320, 408 },
    { 152, 172, 16, 16, 21, 208, 16, 320, 408 },
    { 152, 172, 16, 16, 21, 224, 16, 320, 408 },
    { 83, 73, 168, 16, 22, 0, 24, 352, 489 },
    { 106, 157, 112, 16, 21, 0, 217, 352, 489 },
    { 83, 65, 160, 32, 53, 0, 153, 320, 489 },
    { 106, 149, 112, 32, 53, 0, 185, 320, 489 },
    { 0, 201, 256, 8, 22, 0, 0, 320, 242 },
    { 0, 92, 160, 25, 26, 64, 224, 16, 483 },
    { 160, 92, 160, 25, 26, 64, 224, 16, 483 },
    { 34, 127, 51, 25, 26, 192, 200, 16, 483 },
    { 83, 95, 256, 24, 27, 0, 112, 16, 481 },
    { 40, 130, 256, 24, 27, 0, 136, 16, 481 },
    { 243, 130, 256, 24, 27, 0, 160, 16, 481 },
};
u_short gKanaGridX[10][10] = {
    { 354, 354, 353, 353, 352, 351, 350, 349, 65535, 65535 },
    { 313, 310, 307, 303, 299, 295, 290, 283, 277, 65535 },
    { 272, 267, 262, 256, 250, 242, 234, 224, 213, 199 },
    { 235, 229, 222, 214, 205, 195, 184, 171, 156, 138 },
    { 202, 194, 185, 175, 165, 153, 139, 124, 106, 86 },
    { 170, 160, 150, 139, 126, 113, 97, 80, 60, 38 },
    { 137, 127, 116, 104, 90, 74, 58, 39, 17, 65528 },
    { 109, 98, 85, 72, 57, 40, 22, 1, 65515, 65488 },
    { 84, 71, 58, 43, 27, 9, 65525, 65504, 65535, 65535 },
    { 58, 45, 30, 14, 65534, 65516, 65495, 65535, 65535, 65535 },
};
u_char gKanaGridY[10][10] = {
    { 99, 111, 125, 140, 157, 177, 200, 227, 255, 0 },
    { 92, 103, 115, 130, 145, 163, 184, 208, 237, 0 },
    { 85, 95, 107, 120, 134, 151, 170, 192, 217, 246 },
    { 78, 88, 99, 111, 125, 140, 157, 177, 200, 226 },
    { 72, 81, 92, 103, 116, 130, 146, 164, 185, 208 },
    { 67, 75, 85, 96, 107, 120, 135, 152, 170, 192 },
    { 61, 69, 78, 88, 99, 111, 125, 140, 157, 177 },
    { 56, 64, 72, 81, 91, 103, 116, 130, 146, 164 },
    { 52, 59, 67, 76, 85, 96, 108, 121, 0, 0 },
    { 47, 54, 61, 70, 78, 89, 100, 0, 0, 0 },
};
u_char gKanaCellShades[84] = {
    32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40,
    40, 40, 40, 40, 40, 40, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 64, 80, 80, 96, 96, 64, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 40, 40, 40,
    40, 40, 40, 40, 40, 40, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 0,
};
u_char gKanaTilePositions[8][2] = {
    { 135, 99 },
    { 164, 77 },
    { 205, 77 },
    { 234, 99 },
    { 234, 131 },
    { 205, 153 },
    { 164, 153 },
    { 135, 131 },
};

u_char gPlayerName[56];
