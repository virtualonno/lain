/* Bitmap font renderer and animated grid caption bar: renders 24- and 16-pixel text into VRAM
 * (UploadTextToVram, enum TextUploadMode) and drives the cell caption states (gCaptionState,
 * enum CaptionState) from BIN.BIN parts 0, 1, and 9. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <memory.h>
#include <strings.h>
#include "loading.h"
#include "nodes.h"
#include "cd.h"
#include "graphics.h"
#include "grid_menu.h"
#include "grid_nodes.h"
#include "heap.h"
#include "file_parts.h"
#include "lapks_player.h"
#include "sound.h"
#include "string_tables.h"
#include "text.h"

short gCaptionMoving = 0;
short gShowAllTitles = 0;
u_char *gFontPixels;
enum CaptionState {
    CAPTION_IDLE,
    CAPTION_LEAVING,
    CAPTION_ARRIVING
};
short gCaptionState;
enum CaptionDisplay {
    CAPTION_DISPLAY_FULL,
    CAPTION_DISPLAY_TEXT_ONLY,
    CAPTION_DISPLAY_MOVE
};
short gCaptionTextOnly;
short gCaptionFrame;
short gCaptionLength;
short gCaptionX;
short gCaptionY;
short gCaptionTargetX;
short gCaptionTargetY;
short gCaptionStyle;
short gCaptionWidth;

static u_long sFontTim[4384];
static u_char sTextImage[3072];
static short sGlyphWidths[16];
static short sGlyphDelays[16];
static short sGlyphSteps[16];
static char sSavedCaptionText[16];
static u_char sCaptionText[176];
static GsSPRITE sGlyphSprites[16];
static GsSPRITE sLowerBar;
static GsSPRITE sUpperBar;
static GsSPRITE sTitlePlates[4];
static GsSPRITE sTitleSprites[4];
static GsSPRITE sNoSharedNodePanel[3];

int CopyLargeGlyph(int pos, int c, int fixed)
{
    int band;
    int width;
    int height;
    int col;
    unsigned int packedInfo;
    u_char *dst, *src;

    if (c >= 128) {
        return 0;
    }

    packedInfo = gLargeFontGlyphs[c][1];
    c = gLargeFontGlyphs[c][0];
    band = packedInfo & 7;
    width = packedInfo >> 3;
    if (fixed) {
        dst = sTextImage + pos * 8;
        if (width > 16) {
            width = 16;
        }
    } else {
        dst = sTextImage + pos / 2;
    }

    if (width != 0) {
        c /= 2;
        width++;
        if (band < 2) {
            pos = band * 16;
            height = 16;
        } else {
            pos = (band - 2) * 24 + 32;
            height = 24;
        }
        band = (width + 1) / 2;
        src = gFontPixels + c + pos * 128;
        for (fixed = 0; fixed < height; fixed++) {
            for (col = 0; col < band; col++) {
                dst[col] = src[col];
            }
            src += 128;
            dst += 128;
        }
    }
    return width;
}

void PrepareCaptionSprites(void)
{
    RECT rect;
    GsSPRITE *sprite;
    int glyphCount, i;

    glyphCount = strlen((char *)sCaptionText);
    gCaptionLength = glyphCount;
    setRECT(&rect, 256, 480, 64, 24);
    memset(sTextImage, 0, 0xc00);
    gCaptionWidth = 0;
    for (i = 0; i < glyphCount; i++) {
        gCaptionWidth += sGlyphWidths[i] = CopyLargeGlyph(i, sCaptionText[i], 1);
    }
    DrawSync(0);
    LoadImage(&rect, (u_long *)sTextImage);
    for (i = 0; i < glyphCount; i++) {
        short component;
        u_short glyphWidth = sGlyphWidths[i];
        sprite = &sGlyphSprites[i];
        sprite->h = 24;
        sprite->w = glyphWidth;
        sprite->tpage = GetTPage(0, 0, 256, 480);
        sprite->u = i * 16;
        sprite->v = 224;
        component = (i == 0 || gCaptionStyle != CAPTION_STYLE_CELL) ? 16 : 32;
        sprite->cx = component;
        component = 481;
        sprite->cy = component;
        component = 128;
        sprite->r = component;
        sprite->g = component;
        sprite->b = component;
        sprite->attribute = 0;
        sprite->mx = 0;
        sprite->my = 0;
        component = ONE;
        sprite->scalex = component;
        sprite->scaley = component;
        sprite->rotate = 0;
    }
}

int UploadTextToVram(u_char *text, int x, int y, int mode, int fixedWidth)
{
    RECT rect;

    int length;
    int i;
    int pixelWidth;
    int advance;
    int character;
    u_char *dst;
    unsigned int glyphColumn;
    u_char glyphInfo;
    u_char *src;
    int glyphHeight;
    u_char atlasRow;
    int glyphBand;
    int copyRow;
    int byteColumn;
    u_char (*glyphTable)[2];

    memset(sTextImage, 0, 0xc00);
    length = strlen(text);
    pixelWidth = 0;
    for (i = 0; i < length; i++) {
        glyphTable = gSmallFontGlyphs;
        switch (mode) {
        case TEXT_UPLOAD_LARGE:
            if (fixedWidth != 0) {
                advance = CopyLargeGlyph(i, text[i], fixedWidth);
            } else {
                advance = CopyLargeGlyph(pixelWidth, text[i], 0);
            }
            pixelWidth += advance;
            break;
        case TEXT_UPLOAD_SMALL:
            if (fixedWidth != 0) {
                character = text[i];
                if (character >= 128) {
                    advance = 0;
                } else {
                    dst = sTextImage + i * 4;
                    glyphColumn = glyphTable[character][0];
                    glyphInfo = glyphTable[character][1];
                    glyphHeight = glyphInfo >> 3;
                    glyphBand = glyphInfo & 7;
                    if (glyphHeight != 0) {
                        glyphHeight++;
                        glyphColumn >>= 1;
                        atlasRow = (glyphBand << 3) + 80;
                        src = gFontPixels + glyphColumn + (u_short)(atlasRow << 7);
                        for (copyRow = 0; copyRow < glyphHeight; copyRow++) {
                            for (byteColumn = 0; byteColumn < 4; byteColumn++) {
                                dst[byteColumn] = src[byteColumn];
                            }
                            src += 128;
                            dst += 128;
                        }
                    }
                    advance = 8;
                }
            } else {
                character = text[i];
                if (character >= 128) {
                    advance = 0;
                } else {
                    dst = sTextImage + pixelWidth / 2;
                    glyphColumn = glyphTable[character][0];
                    glyphInfo = glyphTable[character][1];
                    glyphHeight = glyphInfo >> 3;
                    glyphBand = glyphInfo & 7;
                    if (glyphHeight != 0) {
                        glyphHeight++;
                        glyphColumn >>= 1;
                        atlasRow = (glyphBand << 3) + 80;
                        src = gFontPixels + glyphColumn + (u_short)(atlasRow << 7);
                        for (copyRow = 0; copyRow < glyphHeight; copyRow++) {
                            for (byteColumn = 0; byteColumn < 4; byteColumn++) {
                                dst[byteColumn] = src[byteColumn];
                            }
                            src += 128;
                            dst += 128;
                        }
                    }
                    advance = 8;
                }
            }
            pixelWidth += advance;
            break;
        default:
            continue;
        }
        if (pixelWidth & 1) {
            pixelWidth++;
        }
    }
    rect.x = x;
    rect.y = y;
    rect.w = 64;
    switch (mode) {
    case TEXT_UPLOAD_LARGE:
        rect.h = 24;
        break;
    case TEXT_UPLOAD_SMALL:
        rect.h = 16;
        break;
    case TEXT_UPLOAD_CLEAR:
        rect.h = 8;
        break;
    default:
        rect.h = 0;
        break;
    }
    DrawSync(0);
    LoadImage(&rect, (u_long *)sTextImage);
    return pixelWidth;
}

void SetCellCaption(int level, int cell)
{
    int nodeIndex;
    int i;

    nodeIndex = gGridCells[level][cell].nodeIndex;
    if (nodeIndex >= 0) {
        strncpy((char *)sCaptionText, gNodes[nodeIndex].name, 6);
        sCaptionText[6] = '\0';
        PrepareCaptionSprites();
        for (i = 0; i < 4; i++) {
            strncpy(gCaptionTitles[i], gTitles[gNodes[gGridCells[level][cell].nodeIndex].titles[i]], 15);
            gCaptionTitles[i][15] = '\0';
        }
        UploadTextToVram((u_char *)gCaptionTitles[0], 384, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[1], 384, 496, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[2], 448, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[3], 448, 496, TEXT_UPLOAD_SMALL, 1);
    } else {
        gCursorOnNode = 0;
        strcpy((char *)sCaptionText, "Unknown");
        PrepareCaptionSprites();
        for (i = 0; i < 4; i++) {
            strcpy(gCaptionTitles[i], "");
        }
        UploadTextToVram((u_char *)"", 384, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)"", 384, 496, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)"", 448, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)"", 448, 496, TEXT_UPLOAD_SMALL, 1);
    }
}

void InitText(int x, int y)
{
    RECT rect;
    TIM_IMAGE tim;
    int task;
    u_long *first;
    u_long *second;
    u_long *pixels;
    int i;

    task = RequestFilePartRead(3, 1, gBinParts, sFontTim);
    while (PollCdRequest(task) == 0) {
        DrawLoadingFrame(0);
    }
    first = (u_long *)AllocHeapBlock(0x1200);
    task = RequestFilePartRead(3, 0, gBinParts, first);
    while (PollCdRequest(task) == 0) {
        DrawLoadingFrame(0);
    }
    second = (u_long *)AllocHeapBlock(gBinParts[9].size);
    task = RequestFilePartRead(3, 9, gBinParts, second);
    OpenTIM(sFontTim);
    ReadTIM(&tim);
    gFontPixels = (u_char *)tim.paddr;
    LoadClut(tim.caddr, 0, 481);
    gCaptionTextOnly = CAPTION_DISPLAY_FULL;
    gCaptionStyle = CAPTION_STYLE_CELL;
    gCaptionState = CAPTION_IDLE;
    gCaptionX = x;
    gCaptionY = y;
    OpenTIM(first);
    ReadTIM(&tim);
    pixels = tim.paddr;
    LoadClut(tim.caddr, 0, 482);
    setRECT(&rect, 320, 480, 64, 32);
    LoadImage(&rect, pixels);
    while (PollCdRequest(task) == 0) {
        DrawLoadingFrame(0);
    }
    OpenTIM(second);
    ReadTIM(&tim);
    LoadClut(tim.caddr, 0, 484);
    setRECT(&rect, 960, 416, 62, 96);
    LoadImage(&rect, tim.paddr);

    sUpperBar.w = 256;
    sUpperBar.h = 8;
    sUpperBar.tpage = GetTPage(0, 0, 320, 480);
    sUpperBar.u = 0;
    sUpperBar.v = 232;
    sUpperBar.cx = 0;
    sUpperBar.cy = 482;
    sUpperBar.attribute = 0x40000000;
    sUpperBar.r = 128;
    sUpperBar.g = 128;
    sUpperBar.b = 128;
    sUpperBar.mx = sUpperBar.w / 2;
    sUpperBar.my = 0;
    sUpperBar.scalex = 4096;
    sUpperBar.scaley = 4096;
    sUpperBar.rotate = 0;

    sLowerBar.w = 248;
    sLowerBar.h = 8;
    sLowerBar.tpage = GetTPage(0, 0, 320, 480);
    sLowerBar.u = 0;
    sLowerBar.v = 224;
    sLowerBar.cx = 0;
    sLowerBar.cy = 482;
    sLowerBar.attribute = 0x40000000;
    sLowerBar.r = 128;
    sLowerBar.g = 128;
    sLowerBar.b = 128;
    sLowerBar.mx = sLowerBar.w / 2;
    sLowerBar.my = 0;
    sLowerBar.scalex = 4096;
    sLowerBar.scaley = 4096;
    sLowerBar.rotate = 0;

    for (i = 0; i < 4; i++) {
        sTitlePlates[i].w = 136;
        sTitlePlates[i].h = 16;
        sTitlePlates[i].tpage = GetTPage(0, 0, 320, 480);
        sTitlePlates[i].u = 0;
        sTitlePlates[i].v = 240;
        sTitlePlates[i].cx = 0;
        sTitlePlates[i].cy = 482;
        sTitlePlates[i].attribute = 0x40000000;
        sTitlePlates[i].r = 128;
        sTitlePlates[i].g = 128;
        sTitlePlates[i].b = 128;
        sTitlePlates[i].mx = 68;
        sTitlePlates[i].my = 0;
        sTitlePlates[i].scalex = 4096;
        sTitlePlates[i].scaley = 4096;
        sTitlePlates[i].rotate = 0;

        sTitleSprites[i].w = 120;
        sTitleSprites[i].h = 16;
        sTitleSprites[i].u = 0;
        sTitleSprites[i].cx = 0;
        sTitleSprites[i].cy = 481;
        sTitleSprites[i].attribute = 0;
        sTitleSprites[i].r = 128;
        sTitleSprites[i].g = 128;
        sTitleSprites[i].b = 128;
        sTitleSprites[i].mx = 0;
        sTitleSprites[i].my = 0;
        sTitleSprites[i].scalex = 4096;
        sTitleSprites[i].scaley = 4096;
        sTitleSprites[i].rotate = 0;
    }
    sTitleSprites[0].tpage = 22;
    sTitleSprites[0].v = 224;
    sTitleSprites[1].tpage = 22;
    sTitleSprites[1].v = 240;
    sTitleSprites[2].tpage = 23;
    sTitleSprites[2].v = 224;
    sTitleSprites[3].tpage = 23;
    sTitleSprites[3].v = 240;
    FreeHeapBlock(first);
    FreeHeapBlock(second);
}

void DrawCaption(void)
{
    int oppositeSide;
    short x;
    short phaseRemainder;
    int i;
    int glyphX = gCaptionX;
    int glyphY = gCaptionY;

    oppositeSide = gCursorColumn - gGridHeading;
    if (oppositeSide < 0) {
        oppositeSide += 8;
    }
    if (oppositeSide == 3) {
        oppositeSide = 1;
    } else {
        oppositeSide = 0;
    }
    if (gCaptionTextOnly == CAPTION_DISPLAY_FULL) {
        x = gCaptionX;
        if (x < 160) {
            sLowerBar.x = x - 156;
            sUpperBar.x = x + 77;
            sLowerBar.attribute &= ~0x800000;
            sUpperBar.attribute &= ~0x800000;
            sLowerBar.y = gCaptionY + 18;
            sUpperBar.y = gCaptionY + 12;
            GsSortFastSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            GsSortFastSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 4);
        } else {
            sLowerBar.attribute |= 0x800000;
            sUpperBar.attribute |= 0x800000;
            sLowerBar.x = x - 161 + sLowerBar.w / 2;
            sLowerBar.y = gCaptionY - 102;
            sUpperBar.x = x - 408 + sUpperBar.w / 2;
            sUpperBar.y = gCaptionY - 108;
            GsSortSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            GsSortSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 4);
        }
        if (gShowAllTitles != 0) {
            for (i = 0; i < 4; i++) {
                if (gCaptionX < 160) {
                    sTitlePlates[i].attribute &= ~0x800000;
                    phaseRemainder = gCursorRow % 3;
                    if (phaseRemainder > 0 || oppositeSide) {
                        unsigned short panelY = gCaptionY;
                        sTitlePlates[i].x = 181 - i * 6;
                        sTitlePlates[i].y = i * 17 + (short)(panelY + 20);
                        sTitleSprites[i].x = 188 - i * 6;
                        sTitleSprites[i].y = i * 17 + (short)(panelY + 22);
                    } else {
                        unsigned short panelY = gCaptionY;
                        sTitlePlates[i].x = 181 - i * 6;
                        sTitlePlates[i].y = i * 17 + (short)(panelY - 47);
                        sTitleSprites[i].x = 188 - i * 6;
                        sTitleSprites[i].y = i * 17 + (short)(panelY - 45);
                    }
                    GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortFastSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                } else {
                    sTitlePlates[i].attribute |= 0x800000;
                    phaseRemainder = gCursorRow % 3;
                    if (phaseRemainder > 0 || oppositeSide) {
                        unsigned short panelY = gCaptionY;
                        sTitlePlates[i].x = i * 6 - 88;
                        sTitlePlates[i].y = panelY + i * 17 - 100;
                        sTitleSprites[i].x = i * 6 + 13;
                        sTitleSprites[i].y = i * 17 + (short)(panelY + 22);
                    } else {
                        unsigned short panelY = gCaptionY;
                        sTitlePlates[i].x = i * 6 - 88;
                        sTitlePlates[i].y = panelY + i * 17 - 167;
                        sTitleSprites[i].x = i * 6 + 13;
                        sTitleSprites[i].y = i * 17 + (short)(panelY - 45);
                    }
                    GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                }
            }
        } else {
            if (gCaptionX < 160) {
                sTitlePlates[0].attribute &= ~0x800000;
                phaseRemainder = gCursorRow % 3;
                if (phaseRemainder > 0 || oppositeSide) {
                    sTitlePlates[0].x = 181;
                    sTitleSprites[0].x = 188;
                    sTitlePlates[0].y = gCaptionY + 20;
                    sTitleSprites[0].y = gCaptionY + 22;
                } else {
                    sTitlePlates[0].x = 163;
                    sTitleSprites[0].x = 170;
                    sTitlePlates[0].y = gCaptionY + 4;
                    sTitleSprites[0].y = gCaptionY + 6;
                }
                GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortFastSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
            } else {
                sTitlePlates[0].attribute |= 0x800000;
                phaseRemainder = gCursorRow % 3;
                if (phaseRemainder > 0 || oppositeSide) {
                    sTitlePlates[0].x = -88;
                    sTitleSprites[0].x = 13;
                    sTitlePlates[0].y = gCaptionY - 100;
                    sTitleSprites[0].y = gCaptionY + 22;
                } else {
                    sTitlePlates[0].x = -70;
                    sTitleSprites[0].x = 31;
                    sTitlePlates[0].y = gCaptionY - 116;
                    sTitleSprites[0].y = gCaptionY + 6;
                }
                GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
            }
        }
    }
    for (i = 0; i < gCaptionLength; i++) {
        sGlyphSprites[i].x = glyphX;
        sGlyphSprites[i].y = glyphY;
        glyphX += sGlyphWidths[i];
        GsSortFastSprite(&sGlyphSprites[i], &gSubOt[gActiveBuffer], i + 2);
    }
}

void DrawCaptionLeaving(void)
{
    int showAlternateRow;
    short anchorX;
    short anchorY;
    short gridRemainder;
    int i;
    int textWidth;
    short slideOffset;

    showAlternateRow = gPreviousCursorRow - gGridHeading;
    if (showAlternateRow < 0) {
        showAlternateRow += 8;
    }
    if (showAlternateRow == 3) {
        showAlternateRow = 1;
    } else {
        showAlternateRow = 0;
    }
    if (gCaptionTextOnly == CAPTION_DISPLAY_FULL && gCaptionFrame < 4) {
        anchorX = gCaptionX;
        if (anchorX < 160) {
            sLowerBar.attribute &= ~0x800000;
            sUpperBar.attribute &= ~0x800000;
            slideOffset = (-92 - anchorX) * gCaptionFrame / 4 - 156;
            sLowerBar.x = anchorX + slideOffset;
            sLowerBar.y = gCaptionY + 18;
            slideOffset = (243 - anchorX) * gCaptionFrame / 4 + 77;
            sUpperBar.x = anchorX + slideOffset;
            sUpperBar.y = gCaptionY + 12;
            GsSortFastSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
            GsSortFastSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
        } else {
            sLowerBar.attribute |= 0x800000;
            sUpperBar.attribute |= 0x800000;
            slideOffset = (321 - anchorX) * gCaptionFrame / 4 - 1;
            sLowerBar.x = anchorX + slideOffset + (short)(sLowerBar.w / 2 - 160);
            sLowerBar.y = gCaptionY - 102;
            slideOffset = (-8 - anchorX) * gCaptionFrame / 4 - 248;
            sUpperBar.y = gCaptionY - 108;
            sUpperBar.x = anchorX + slideOffset + (short)(sUpperBar.w / 2 - 160);
            GsSortSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
            GsSortSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
        }
        if (gShowAllTitles != 0) {
            for (i = 0; i < 4; i++) {
                if (gCaptionX < 160) {
                    sTitlePlates[i].attribute &= ~0x800000;
                    gridRemainder = gPreviousCursorRow % 3;
                    if (gridRemainder > 0 || showAlternateRow) {
                        sTitlePlates[i].x = 181 - i * 6 + gCaptionFrame * 150 / 4;
                        anchorY = gCaptionY;
                        sTitlePlates[i].y = 20 + i * 17 + anchorY;
                        sTitleSprites[i].x = 188 - i * 6 + gCaptionFrame * 150 / 4;
                        sTitleSprites[i].y = 22 + i * 17 + anchorY;
                    } else {
                        sTitlePlates[i].x = 181 - i * 6 + gCaptionFrame * 150 / 4;
                        anchorY = gCaptionY;
                        sTitlePlates[i].y = -47 + i * 17 + anchorY;
                        sTitleSprites[i].x = 188 - i * 6 + gCaptionFrame * 150 / 4;
                        sTitleSprites[i].y = -45 + i * 17 + anchorY;
                    }
                    GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortFastSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                } else {
                    sTitlePlates[i].attribute |= 0x800000;
                    gridRemainder = gPreviousCursorRow % 3;
                    if (gridRemainder > 0 || showAlternateRow) {
                        sTitlePlates[i].x = i * 6 - 88 - gCaptionFrame * 158 / 4;
                        anchorY = gCaptionY;
                        sTitlePlates[i].y = i * 17 + anchorY - 100;
                        sTitleSprites[i].x = i * 6 + 13 - gCaptionFrame * 158 / 4;
                        sTitleSprites[i].y = 22 + i * 17 + anchorY;
                    } else {
                        sTitlePlates[i].x = i * 6 - 88 - gCaptionFrame * 158 / 4;
                        anchorY = gCaptionY;
                        sTitlePlates[i].y = i * 17 + anchorY - 167;
                        sTitleSprites[i].x = i * 6 + 13 - gCaptionFrame * 158 / 4;
                        sTitleSprites[i].y = -45 + i * 17 + anchorY;
                    }
                    GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                }
            }
        } else {
            if (gCaptionX < 160) {
                sTitlePlates[0].attribute &= ~0x800000;
                gridRemainder = gPreviousCursorRow % 3;
                if (gridRemainder > 0 || showAlternateRow) {
                    sTitlePlates[0].x = (gCaptionFrame * 150 / 4) + 181;
                    anchorY = gCaptionY;
                    sTitlePlates[0].y = anchorY + 20;
                    sTitleSprites[0].x = (gCaptionFrame * 150 / 4) + 188;
                    sTitleSprites[0].y = anchorY + 22;
                } else {
                    sTitlePlates[0].x = (gCaptionFrame * 150 / 4) + 163;
                    anchorY = gCaptionY;
                    sTitlePlates[0].y = anchorY + 4;
                    sTitleSprites[0].x = (gCaptionFrame * 150 / 4) + 170;
                    sTitleSprites[0].y = anchorY + 6;
                }
                GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortFastSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
            } else {
                sTitlePlates[0].attribute |= 0x800000;
                gridRemainder = gPreviousCursorRow % 3;
                if (gridRemainder > 0 || showAlternateRow) {
                    sTitlePlates[0].x = - 88 - (gCaptionFrame * 158 / 4);
                    anchorY = gCaptionY;
                    sTitlePlates[0].y = anchorY - 100;
                    sTitleSprites[0].x = 13 - (gCaptionFrame * 158 / 4);
                    sTitleSprites[0].y = anchorY + 22;
                } else {
                    sTitlePlates[0].x = -70 - (gCaptionFrame * 158 / 4);
                    anchorY = gCaptionY;
                    sTitlePlates[0].y = anchorY - 116;
                    sTitleSprites[0].x = 31 - (gCaptionFrame * 158 / 4);
                    sTitleSprites[0].y = anchorY + 6;
                }
                GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
            }
        }
    }
    if (sGlyphSteps[0] >= 4) {
        sGlyphSprites[0].x = gCaptionTargetX;
        sGlyphSprites[0].y = gCaptionTargetY;
    } else {
        sGlyphSprites[0].x = gCaptionX + (gCaptionTargetX - gCaptionX) * sGlyphSteps[0] / 4;
        sGlyphSprites[0].y = gCaptionY + (gCaptionTargetY - gCaptionY) * sGlyphSteps[0] / 4;
    }
    sGlyphSteps[0]++;
    GsSortFastSprite(&sGlyphSprites[0], &gSubOt[gActiveBuffer], 2);
    textWidth = sGlyphWidths[0];
    for (i = 1; i < gCaptionLength; i++) {
        if (sGlyphDelays[i] != 0) {
            sGlyphDelays[i]--;
            for (; i < gCaptionLength; i++) {
                sGlyphSprites[i].y = gCaptionY;
                sGlyphSprites[i].x = gCaptionX + textWidth;
                GsSortFastSprite(&sGlyphSprites[i], &gSubOt[gActiveBuffer], i + 2);
                textWidth += sGlyphWidths[i];
            }
            break;
        }
        if (sGlyphSteps[i] >= 4) {
            sGlyphSprites[i].x = gCaptionTargetX;
            sGlyphSprites[i].y = gCaptionTargetY;
        } else {
            sGlyphSprites[i].x = gCaptionX + (gCaptionTargetX - gCaptionX) * sGlyphSteps[i] / 4;
            sGlyphSprites[i].y = gCaptionY + (gCaptionTargetY - gCaptionY) * sGlyphSteps[i] / 4;
        }
        sGlyphSteps[i]++;
        GsSortFastSprite(&sGlyphSprites[i], &gSubOt[gActiveBuffer], i + 2);
        textWidth += sGlyphWidths[i];
    }
    gCaptionFrame++;
    if (sGlyphSteps[gCaptionLength - 1] >= 5) {
        if (gCaptionTextOnly == CAPTION_DISPLAY_MOVE) {
            gCaptionTextOnly = CAPTION_DISPLAY_FULL;
        }
        gCaptionState = CAPTION_ARRIVING;
        gCaptionFrame = 0;
        gCaptionX = gCaptionTargetX;
        gCaptionY = gCaptionTargetY;
        strcpy((char *)sCaptionText, gNextCaptionText);
        if (gCaptionStyle == CAPTION_STYLE_PLAIN) {
            gCaptionTextOnly = CAPTION_DISPLAY_TEXT_ONLY;
        } else if (gCaptionStyle == CAPTION_STYLE_RESTORE_CELL) {
            gCaptionTextOnly = CAPTION_DISPLAY_FULL;
            gCaptionStyle = CAPTION_STYLE_CELL;
        }
        PrepareCaptionSprites();
        UploadTextToVram((u_char *)gCaptionTitles[0], 384, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[1], 384, 496, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[2], 448, 480, TEXT_UPLOAD_SMALL, 1);
        UploadTextToVram((u_char *)gCaptionTitles[3], 448, 496, TEXT_UPLOAD_SMALL, 1);
        for (i = 0; i < gCaptionLength; i++) {
            sGlyphDelays[i] = 1;
            sGlyphSteps[i] = 0;
        }
        StopSound();
    }
}

void DrawCaptionArriving(void)
{
    int flag;
    short x;
    short y;
    int rowX;
    int rowY;
    int phase;
    int loopTravelFrames;
    int singleTravelFrames;
    short rem;
    int i;

    flag = gCursorColumn - gGridHeading;
    if (flag < 0) {
        flag += 8;
    }
    if (flag == 3) {
        flag = 1;
    } else {
        flag = 0;
    }
    if (gCaptionTextOnly == CAPTION_DISPLAY_FULL) {
        x = gCaptionX;
        if (x < 160) {
            if (gCaptionFrame < 4) {
                sLowerBar.attribute &= ~0x800000;
                sUpperBar.attribute &= ~0x800000;
                sLowerBar.x = (x + 92) * gCaptionFrame / 4 - 248;
                sLowerBar.y = gCaptionY + 18;
                sUpperBar.x = (x - 243) * gCaptionFrame / 4 + 320;
                sUpperBar.y = gCaptionY + 12;
                GsSortFastSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortFastSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            } else {
                sLowerBar.x = x - 156;
                sUpperBar.x = x + 77;
                sLowerBar.attribute &= ~0x800000;
                sUpperBar.attribute &= ~0x800000;
                sLowerBar.y = gCaptionY + 18;
                sUpperBar.y = gCaptionY + 12;
                GsSortFastSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortFastSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            }
        } else {
            if (gCaptionFrame < 4) {
                sLowerBar.attribute |= 0x800000;
                sUpperBar.attribute |= 0x800000;
                sLowerBar.x = (x - 321) * gCaptionFrame / 4 + 160 + sLowerBar.w / 2;
                sLowerBar.y = gCaptionY - 102;
                sUpperBar.x = (x + 8) * gCaptionFrame / 4 - 416 + sUpperBar.w / 2;
                sUpperBar.y = gCaptionY - 108;
                GsSortSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            } else {
                sLowerBar.attribute |= 0x800000;
                sUpperBar.attribute |= 0x800000;
                sLowerBar.x = x - 161 + sLowerBar.w / 2;
                sLowerBar.y = gCaptionY - 102;
                sUpperBar.x = x - 408 + sUpperBar.w / 2;
                sUpperBar.y = gCaptionY - 108;
                GsSortSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
                GsSortSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 3);
            }
        }
        if (gShowAllTitles != 0) {
            for (i = 0; i < 4; i++) {
                if (gCaptionFrame < 4) {
                    if (gCaptionX < 160) {
                        sTitlePlates[i].attribute &= ~0x800000;
                        rem = gCursorRow % 3;
                        if (rem > 0 || flag) {
                            sTitlePlates[i].x = 181 - i * 6 + ((4 - gCaptionFrame) * 150 / 4);
                            y = gCaptionY;
                            sTitlePlates[i].y = 20 + i * 17 + y;
                            sTitleSprites[i].x = 188 - i * 6 + ((4 - gCaptionFrame) * 150 / 4);
                            sTitleSprites[i].y = 22 + i * 17 + y;
                        } else {
                            sTitlePlates[i].x = 181 - i * 6 + ((4 - gCaptionFrame) * 150 / 4);
                            y = gCaptionY;
                            sTitlePlates[i].y = -47 + i * 17 + y;
                            sTitleSprites[i].x = 188 - i * 6 + ((4 - gCaptionFrame) * 150 / 4);
                            sTitleSprites[i].y = -45 + i * 17 + y;
                        }
                        GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                        GsSortFastSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    } else {
                        sTitlePlates[i].attribute |= 0x800000;
                        loopTravelFrames = 4;
                        rem = gCursorRow % 3;
                        if (rem > 0 || flag) {
                            sTitlePlates[i].x = i * 6 - 88 - ((loopTravelFrames - gCaptionFrame) * 158 / 4);
                            y = gCaptionY;
                            sTitlePlates[i].y = y + i * 17 - 100;
                            sTitleSprites[i].x = i * 6 + 13 - ((loopTravelFrames - gCaptionFrame) * 158 / 4);
                            sTitleSprites[i].y = 22 + i * 17 + y;
                        } else {
                            sTitlePlates[i].x = i * 6 - 88 - ((loopTravelFrames - gCaptionFrame) * 158 / 4);
                            y = gCaptionY;
                            sTitlePlates[i].y = y + i * 17 - 167;
                            sTitleSprites[i].x = i * 6 + 13 - ((loopTravelFrames - gCaptionFrame) * 158 / 4);
                            sTitleSprites[i].y = -45 + i * 17 + y;
                        }
                        GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                        GsSortSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    }
                } else {
                    if (gCaptionX < 160) {
                        sTitlePlates[i].attribute &= ~0x800000;
                        rem = gCursorRow % 3;
                        if (rem > 0 || flag) {
                            sTitlePlates[i].x = 181 - i * 6;
                            sTitlePlates[i].y = 20 + i * 17 + gCaptionY;
                            sTitleSprites[i].x = 188 - i * 6;
                            sTitleSprites[i].y = 22 + i * 17 + gCaptionY;
                        } else {
                            sTitlePlates[i].x = 181 - i * 6;
                            sTitlePlates[i].y = -47 + i * 17 + gCaptionY;
                            sTitleSprites[i].x = 188 - i * 6;
                            sTitleSprites[i].y = -45 + i * 17 + gCaptionY;
                        }
                        GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                        GsSortFastSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    } else {
                        sTitlePlates[i].attribute |= 0x800000;
                        rem = gCursorRow % 3;
                        if (rem > 0 || flag) {
                            sTitlePlates[i].x = i * 6 - 88;
                            sTitlePlates[i].y = gCaptionY + i * 17 - 100;
                            sTitleSprites[i].x = i * 6 + 13;
                            sTitleSprites[i].y = 22 + i * 17 + gCaptionY;
                        } else {
                            sTitlePlates[i].x = i * 6 - 88;
                            sTitlePlates[i].y = gCaptionY + i * 17 - 167;
                            sTitleSprites[i].x = i * 6 + 13;
                            sTitleSprites[i].y = -45 + i * 17 + gCaptionY;
                        }
                        GsSortFastSprite(&sTitleSprites[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                        GsSortSprite(&sTitlePlates[i], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    }
                }
            }
        } else {
            if (gCaptionFrame < 4) {
                if (gCaptionX < 160) {
                    sTitlePlates[0].attribute &= ~0x800000;
                    rem = gCursorRow % 3;
                    if (rem > 0 || flag) {
                        sTitlePlates[0].x = ((4 - gCaptionFrame) * 150 / 4) + 181;
                        y = gCaptionY;
                        sTitlePlates[0].y = y + 20;
                        sTitleSprites[0].x = ((4 - gCaptionFrame) * 150 / 4) + 188;
                        sTitleSprites[0].y = y + 22;
                    } else {
                        sTitlePlates[0].x = ((4 - gCaptionFrame) * 150 / 4) + 163;
                        y = gCaptionY;
                        sTitlePlates[0].y = y + 4;
                        sTitleSprites[0].x = ((4 - gCaptionFrame) * 150 / 4) + 170;
                        sTitleSprites[0].y = y + 6;
                    }
                    GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortFastSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                } else {
                    sTitlePlates[0].attribute |= 0x800000;
                    rem = gCursorRow % 3;
                    if (rem > 0 || (singleTravelFrames = 4, flag)) {
                        sTitlePlates[0].x = -88 - ((4 - gCaptionFrame) * 158 / 4);
                        y = gCaptionY;
                        sTitlePlates[0].y = y - 100;
                        sTitleSprites[0].x = 13 - ((4 - gCaptionFrame) * 158 / 4);
                        sTitleSprites[0].y = y + 22;
                    } else {
                        singleTravelFrames -= gCaptionFrame;
                        sTitlePlates[0].x = -70 - ((singleTravelFrames) * 158 / 4);
                        y = gCaptionY;
                        sTitlePlates[0].y = y - 116;
                        sTitleSprites[0].x = 31 - ((singleTravelFrames) * 158 / 4);
                        sTitleSprites[0].y = y + 6;
                    }
                    GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                }
            } else {
                if (gCaptionX < 160) {
                    sTitlePlates[0].attribute &= ~0x800000;
                    rem = gCursorRow % 3;
                    if (rem > 0 || flag) {
                        sTitlePlates[0].x = 181;
                        sTitlePlates[0].y = gCaptionY + 20;
                        sTitleSprites[0].x = 188;
                        sTitleSprites[0].y = gCaptionY + 22;
                    } else {
                        sTitlePlates[0].x = 163;
                        sTitlePlates[0].y = gCaptionY + 4;
                        sTitleSprites[0].x = 170;
                        sTitleSprites[0].y = gCaptionY + 6;
                    }
                    GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortFastSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                } else {
                    sTitlePlates[0].attribute |= 0x800000;
                    rem = gCursorRow % 3;
                    if (rem > 0 || flag) {
                        sTitlePlates[0].x = -88;
                        sTitlePlates[0].y = gCaptionY - 100;
                        sTitleSprites[0].x = 13;
                        sTitleSprites[0].y = gCaptionY + 22;
                    } else {
                        sTitlePlates[0].x = -70;
                        sTitlePlates[0].y = gCaptionY - 116;
                        sTitleSprites[0].x = 31;
                        sTitleSprites[0].y = gCaptionY + 6;
                    }
                    GsSortFastSprite(&sTitleSprites[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                    GsSortSprite(&sTitlePlates[0], &gSubOt[gActiveBuffer], gCaptionLength + 2);
                }
            }
        }
    }
    rowX = sGlyphSprites[0].x = gCaptionTargetX;
    rowY = sGlyphSprites[0].y = gCaptionTargetY;
    GsSortFastSprite(&sGlyphSprites[0], &gSubOt[gActiveBuffer], 2);
    for (i = 1; i < gCaptionLength; i++) {
        if (sGlyphDelays[i] != 0) {
            sGlyphDelays[i]--;
            for (; i < gCaptionLength; i++) {
                sGlyphSprites[i].x = rowX;
                sGlyphSprites[i].y = rowY;
                GsSortFastSprite(&sGlyphSprites[i], &gSubOt[gActiveBuffer], i + 2);
            }
            break;
        }
        phase = sGlyphSteps[i];
        if (phase >= 2) {
            sGlyphSprites[i].x = rowX + sGlyphWidths[i];
            sGlyphSprites[i].y = rowY;
            rowX = sGlyphSprites[i].x;
        } else {
            sGlyphSprites[i].x = rowX + sGlyphWidths[i] * phase / 2;
            sGlyphSprites[i].y = rowY;
            rowX = sGlyphSprites[i].x;
        }
        GsSortFastSprite(&sGlyphSprites[i], &gSubOt[gActiveBuffer], i + 2);
        sGlyphSteps[i]++;
    }
    gCaptionFrame++;
    if (sGlyphSteps[gCaptionLength - 1] > 2 && gCaptionFrame > 4) {
        gCaptionState = CAPTION_IDLE;
        gCaptionFrame = 0;
        gCaptionMoving = 0;
    }
}

void DrawCaptionDuringMove(short x, short y, int commit)
{
    int i;

    gCaptionTextOnly = CAPTION_DISPLAY_MOVE;
    gCaptionMoving = 1;
    gCaptionX = x;
    gCaptionY = y;
    if (x < 160) {
        if (gCaptionFrame < 4) {
            sLowerBar.x = x + gCaptionWidth - (x + gCaptionWidth + 14) * gCaptionFrame / 4 - 210;
            sLowerBar.y = y + 7;
            GsSortFastSprite(&sLowerBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
        }
    } else {
        if (gCaptionFrame < 4) {
            sUpperBar.x = x + (short)((334 - x) * gCaptionFrame / 4 - 14);
            sUpperBar.y = y + 7;
            GsSortFastSprite(&sUpperBar, &gSubOt[gActiveBuffer], gCaptionLength + 2);
        }
    }
    gCaptionFrame++;
    DrawCaption();
    if (commit) {
        gCaptionState = CAPTION_LEAVING;
        gCaptionFrame = 0;
        gCaptionMoving = 1;
        for (i = 0; i < gCaptionLength; i++) {
            sGlyphDelays[i] = 1;
            sGlyphSteps[i] = 0;
        }
    }
}

void DrawNoSharedNodePanel(void)
{
    int i;

    sNoSharedNodePanel[0].x = 0;
    sNoSharedNodePanel[0].y = 111;
    sNoSharedNodePanel[0].w = 160;
    sNoSharedNodePanel[0].h = 48;
    sNoSharedNodePanel[0].tpage = GetTPage(0, 0, 960, 416);
    sNoSharedNodePanel[0].u = 0;
    sNoSharedNodePanel[0].v = 160;
    sNoSharedNodePanel[0].cx = 16;
    sNoSharedNodePanel[0].cy = 484;
    sNoSharedNodePanel[0].attribute = 0x40000000;
    sNoSharedNodePanel[0].r = 128;
    sNoSharedNodePanel[0].g = 128;
    sNoSharedNodePanel[0].b = 128;
    sNoSharedNodePanel[0].mx = 0;
    sNoSharedNodePanel[0].my = 0;
    sNoSharedNodePanel[0].scalex = 4096;
    sNoSharedNodePanel[0].scaley = 4096;
    sNoSharedNodePanel[0].rotate = 0;

    sNoSharedNodePanel[1].x = 160;
    sNoSharedNodePanel[1].y = 111;
    sNoSharedNodePanel[1].w = 160;
    sNoSharedNodePanel[1].h = 48;
    sNoSharedNodePanel[1].tpage = GetTPage(0, 0, 960, 416);
    sNoSharedNodePanel[1].u = 0;
    sNoSharedNodePanel[1].v = 208;
    sNoSharedNodePanel[1].cx = 16;
    sNoSharedNodePanel[1].cy = 484;
    sNoSharedNodePanel[1].attribute = 0x40000000;
    sNoSharedNodePanel[1].r = 128;
    sNoSharedNodePanel[1].g = 128;
    sNoSharedNodePanel[1].b = 128;
    sNoSharedNodePanel[1].mx = 0;
    sNoSharedNodePanel[1].my = 0;
    sNoSharedNodePanel[1].scalex = 4096;
    sNoSharedNodePanel[1].scaley = 4096;
    sNoSharedNodePanel[1].rotate = 0;

    sNoSharedNodePanel[2].x = 42;
    sNoSharedNodePanel[2].y = 117;
    sNoSharedNodePanel[2].w = 88;
    sNoSharedNodePanel[2].h = 24;
    sNoSharedNodePanel[2].tpage = GetTPage(0, 0, 960, 416);
    sNoSharedNodePanel[2].u = 160;
    sNoSharedNodePanel[2].v = 160;
    sNoSharedNodePanel[2].cx = 0;
    sNoSharedNodePanel[2].cy = 484;
    sNoSharedNodePanel[2].attribute = 0x40000000;
    sNoSharedNodePanel[2].r = 128;
    sNoSharedNodePanel[2].g = 128;
    sNoSharedNodePanel[2].b = 128;
    sNoSharedNodePanel[2].mx = 0;
    sNoSharedNodePanel[2].my = 0;
    sNoSharedNodePanel[2].scalex = 4096;
    sNoSharedNodePanel[2].scaley = 4096;
    sNoSharedNodePanel[2].rotate = 0;

    for (i = 2; i >= 0; i--)
    {
        GsSortFastSprite(&sNoSharedNodePanel[i], &gSubOt[gActiveBuffer], 2);
    }
}

short gSavedCaptionX;
short gSavedCaptionY;

void PlaceCellCaption(int x, int y, int level, int cell)
{
    gCaptionX = x;
    gCaptionY = y;
    gCaptionTextOnly = CAPTION_DISPLAY_FULL;
    gCaptionStyle = CAPTION_STYLE_CELL;
    gCaptionState = CAPTION_IDLE;
    SetCellCaption(level, cell);
}

void SetTemporaryCaption(int x, int y, const char *text, int replace)
{
    if (replace == 0) {
        gCaptionX = gSavedCaptionX;
        gCaptionY = gSavedCaptionY;
        strcpy((char *)sCaptionText, sSavedCaptionText);
    } else {
        gSavedCaptionX = gCaptionX;
        gSavedCaptionY = gCaptionY;
        strcpy(sSavedCaptionText, (char *)sCaptionText);
        gCaptionX = x;
        gCaptionY = y;
        strcpy((char *)sCaptionText, text);
    }
    gCaptionTextOnly = replace;
    gCaptionState = CAPTION_IDLE;
    PrepareCaptionSprites();
}

int UpdateCaption(int startTransition)
{
    int i;

    if (startTransition) {
        gCaptionState = CAPTION_LEAVING;
        gCaptionFrame = 0;
        gCaptionMoving = 1;
        for (i = 0; i < gCaptionLength; i++) {
            sGlyphDelays[i] = 1;
            sGlyphSteps[i] = 0;
        }
        StartSound(1);
    }
    switch (gCaptionState) {
    case CAPTION_IDLE:
        DrawCaption();
        break;
    case CAPTION_LEAVING:
        DrawCaptionLeaving();
        break;
    case CAPTION_ARRIVING:
        DrawCaptionArriving();
        break;
    }
}

int SetCaptionStyle(int style)
{
    int valid;

    valid = (unsigned char)(style < 3);
    if (valid != 0 && style > CAPTION_STYLE_CELL) {
        gCaptionStyle = style;
    }
    return valid;
}

u_char gLargeFontGlyphs[128][2] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 42, 60 },
    { 184, 51 },
    { 192, 91 },
    { 204, 139 },
    { 0, 0 },
    { 222, 139 },
    { 0, 132 },
    { 146, 43 },
    { 154, 59 },
    { 162, 59 },
    { 108, 91 },
    { 0, 0 },
    { 92, 43 },
    { 170, 107 },
    { 100, 43 },
    { 28, 100 },
    { 142, 65 },
    { 158, 57 },
    { 168, 65 },
    { 182, 65 },
    { 196, 65 },
    { 210, 65 },
    { 224, 65 },
    { 238, 65 },
    { 0, 66 },
    { 14, 66 },
    { 120, 43 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 18, 76 },
    { 128, 139 },
    { 0, 136 },
    { 18, 96 },
    { 32, 120 },
    { 48, 120 },
    { 64, 88 },
    { 76, 88 },
    { 88, 120 },
    { 104, 120 },
    { 120, 56 },
    { 128, 88 },
    { 140, 112 },
    { 156, 96 },
    { 170, 136 },
    { 188, 112 },
    { 204, 136 },
    { 222, 96 },
    { 236, 136 },
    { 0, 97 },
    { 14, 89 },
    { 26, 121 },
    { 42, 121 },
    { 58, 137 },
    { 76, 169 },
    { 98, 121 },
    { 114, 121 },
    { 130, 89 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 28, 90 },
    { 40, 90 },
    { 52, 82 },
    { 64, 90 },
    { 76, 98 },
    { 90, 74 },
    { 100, 90 },
    { 112, 90 },
    { 124, 50 },
    { 132, 58 },
    { 140, 90 },
    { 152, 50 },
    { 160, 114 },
    { 176, 90 },
    { 188, 98 },
    { 202, 90 },
    { 214, 90 },
    { 226, 74 },
    { 236, 66 },
    { 0, 75 },
    { 10, 91 },
    { 22, 107 },
    { 36, 139 },
    { 54, 91 },
    { 66, 107 },
    { 80, 83 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};
u_char gSmallFontGlyphs[128][2] = {
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x78, 0x3b },
    { 0x30, 0x3b },
    { 0x38, 0x3b },
    { 0x40, 0x3b },
    { 0x00, 0x00 },
    { 0x48, 0x3b },
    { 0x50, 0x3b },
    { 0x10, 0x3b },
    { 0x18, 0x7b },
    { 0x20, 0x7b },
    { 0xd0, 0x3a },
    { 0x00, 0x00 },
    { 0xc0, 0x7a },
    { 0x28, 0x3b },
    { 0xc8, 0x3a },
    { 0x60, 0x7b },
    { 0x58, 0x39 },
    { 0x60, 0x39 },
    { 0x68, 0x39 },
    { 0x70, 0x39 },
    { 0x78, 0x39 },
    { 0x80, 0x39 },
    { 0x88, 0x39 },
    { 0x90, 0x39 },
    { 0x98, 0x39 },
    { 0xa0, 0x39 },
    { 0x00, 0x3b },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x58, 0x3b },
    { 0x08, 0x3b },
    { 0x40, 0x38 },
    { 0x48, 0x38 },
    { 0x50, 0x38 },
    { 0x58, 0x38 },
    { 0x60, 0x38 },
    { 0x68, 0x38 },
    { 0x70, 0x38 },
    { 0x78, 0x38 },
    { 0x80, 0x38 },
    { 0x88, 0x38 },
    { 0x90, 0x38 },
    { 0x98, 0x38 },
    { 0xa0, 0x38 },
    { 0xa8, 0x38 },
    { 0xb0, 0x38 },
    { 0xb8, 0x38 },
    { 0xc0, 0x38 },
    { 0xc8, 0x38 },
    { 0xd0, 0x38 },
    { 0xd8, 0x38 },
    { 0xe0, 0x38 },
    { 0xe8, 0x38 },
    { 0xf0, 0x38 },
    { 0x40, 0x39 },
    { 0x48, 0x39 },
    { 0x50, 0x39 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0xa8, 0x39 },
    { 0xb0, 0x39 },
    { 0xb8, 0x39 },
    { 0xc0, 0x39 },
    { 0xc8, 0x39 },
    { 0xd0, 0x39 },
    { 0xd8, 0x79 },
    { 0xe0, 0x39 },
    { 0xe8, 0x39 },
    { 0xf0, 0x79 },
    { 0x40, 0x3a },
    { 0x48, 0x3a },
    { 0x50, 0x3a },
    { 0x58, 0x3a },
    { 0x60, 0x3a },
    { 0x68, 0x7a },
    { 0x70, 0x7a },
    { 0x78, 0x3a },
    { 0x80, 0x3a },
    { 0x88, 0x3a },
    { 0x90, 0x3a },
    { 0x98, 0x3a },
    { 0xa0, 0x3a },
    { 0xa8, 0x3a },
    { 0xb0, 0x7a },
    { 0xb8, 0x3a },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0x00 },
};

char gNextCaptionText[16];

char gCaptionTitles[4][18];
