#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "sprite_quad.h"

/* Tiles start..start+count-1 of the 7x7 grid spell the item's label, uploaded to VRAM at
 * x 704 + 24 * labelColumn, y 512 - 24 * labelRow. */
typedef struct {
    u_char start;
    u_char count;
    u_char labelColumn;
    u_char labelRow;
} MenuItem;

/* A message's text in its texture page and the rows of window drawn behind it. */
typedef struct {
    u_char u, v, width, height, rows;
} MessageImage;

extern SpriteQuad gMessageWindowQuads[8];
extern GsSPRITE gMessageSprite;
extern u_char gTileNumbers[52];
extern u_char gTileMirror[50][2];
extern MessageImage gMessageImages[17];
extern MenuItem gMenuItems[5];
extern u_char gTileFlipDelay[49];

void InitMessageWindow(void);
/* 1-based indices into gMessageImages; messages 2 and 12 are never shown. */
enum MessageId {
    MESSAGE_NO_CARD = 1,
    MESSAGE_FORMAT_PROMPT = 3,
    MESSAGE_FORMATTING,
    MESSAGE_FORMATTED,
    MESSAGE_INVALID_CARD,
    MESSAGE_NO_FREE_BLOCK,
    MESSAGE_OVERWRITE_PROMPT_OPEN,
    MESSAGE_OVERWRITE_PROMPT,
    MESSAGE_SAVING,
    MESSAGE_SAVED,
    MESSAGE_NO_SAVE = 13,
    MESSAGE_SAVE_CORRUPT,
    MESSAGE_LOADING,
    MESSAGE_LOADED,
    MESSAGE_UNFORMATTED
};

void DrawMessageWindow(int message);
void SetMenuTileImage(int index);
void SetTileGridImage(int index);
void InitMenuQuads(void);
void RenderMenuText(void);
void DrawSystemMenuExcept(int first, int second);
int FlipItemToLabel(int item);
int FlipItemToTiles(int item);
int StepSystemMenuOpen(void);
int StepConfirmOpen(void);
int StepConfirmClose(void);
enum MenuResult {
    MENU_RESULT_CHANGE = -2,
    MENU_RESULT_LOAD,
    MENU_RESULT_STAY,
    MENU_RESULT_EXIT
};

int UpdateSystemMenu(void);
int StepSystemMenuClose(void);
void InitTileGrid(const char *unused, int depth);
void DrawTileGrid(void);
void LoadMenuTileImage(void);
void FreeTileGrid(void);
void SortSpriteQuad(SpriteQuad *entry, GsOT *ot, u_short index);
void SortRotatedSpriteQuad(SpriteQuad *entry, GsOT *ot, u_short index);

#endif
