#ifndef KANA_ENTRY_H
#define KANA_ENTRY_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "sprite_quad.h"

typedef struct {
    short x, y;
    u_short w, h;
    u_short tpage;
    u_char u, v;
    u_short cx, cy;
} SpriteDesc;

/* The kana entry screen's state, kept on RunNameEntry's stack. */
typedef struct {
    POLY_FT4 cells[81];
    POLY_FT4 highlight;
    GsSPRITE sprites[6];
    GsSPRITE glyphs[9];
    SpriteQuad tiles[8];
    char unused[0x1230 - 0x114C]; /* 228 bytes nothing reads */
    short row;
    short col;
    short count;
    u_char text[8];
} KanaEntry;

extern u_long *gStripTim;
extern SpriteDesc gStartScreenSprites[29];
extern u_short gKanaGridX[10][10];
extern u_char gKanaGridY[10][10];
extern u_char gKanaCellShades[84];
extern u_char gKanaTilePositions[8][2];
extern u_char gPlayerName[56];

int RunStartScreen(void);
int InitKanaEntry(KanaEntry *menu);
int RunKanaEntryMenu(KanaEntry *menu);
int RomanizeKanaText(KanaEntry *entry);
int RunNameEntry(void);

#endif
