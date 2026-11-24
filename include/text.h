#ifndef TEXT_H
#define TEXT_H

#include <sys/types.h>

enum TextUploadMode {
    TEXT_UPLOAD_LARGE,
    TEXT_UPLOAD_SMALL,
    TEXT_UPLOAD_CLEAR
};

enum CaptionStyle {
    CAPTION_STYLE_CELL,
    CAPTION_STYLE_PLAIN,
    CAPTION_STYLE_RESTORE_CELL
};

extern short gCaptionMoving;
extern short gShowAllTitles;
extern short gCaptionState;
extern short gCaptionTextOnly;
extern short gCaptionFrame;
extern short gCaptionLength;
extern short gCaptionX;
extern short gCaptionY;
extern short gCaptionTargetX;
extern short gCaptionTargetY;
/* Packed glyph atlas entries: [0] = x pixel offset, [1] = (width/height - 1 << 3) | band. */
extern u_char gLargeFontGlyphs[128][2];
extern u_char gSmallFontGlyphs[128][2];
extern char gNextCaptionText[16];
extern char gCaptionTitles[4][18];

int CopyLargeGlyph(int pos, int c, int fixed);
int UploadTextToVram(u_char *text, int x, int y, int mode, int fixedWidth);
void InitText(int x, int y);
void DrawCaptionDuringMove(short x, short y, int commit);
void DrawNoSharedNodePanel(void);
void PlaceCellCaption(int x, int y, int level, int cell);
void SetTemporaryCaption(int x, int y, const char *text, int replace);
int UpdateCaption(int startTransition);
int SetCaptionStyle(int style);

extern u_char *gFontPixels;

#endif
