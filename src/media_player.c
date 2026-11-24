/* Media node player: streams MOVIE/*.STR video (including EndRoll) and plays XA audio tracks 0..517
 * with the word ring, track images, and Play/Exit controls; hands tracks >= 518 and 733 to voice_screen. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include "sprite_quad.h"

#include <libetc.h>
#include <libgs.h>
#include <libcd.h>
#include <libpress.h>
#include <libspu.h>
#include <rand.h>
#include <memory.h>
#include <strings.h>
#include "loading.h"
#include "nodes.h"
#include "voice_screen.h"
#include "cd.h"
#include "graphics.h"
#include "grid_menu.h"
#include "heap.h"
#include "INIT.h"
#include "DECODE.h"
#include "file_parts.h"
#include "media_player.h"
#include "napk.h"
#include "pad.h"
#include "sound.h"
#include "system_menu.h"
#include "text.h"
#include "media_cube.h"

enum MovieState {
    MOVIE_DECODE_STILL = -1,
    MOVIE_STILL,
    MOVIE_PLAYING
};
short gMovieState = MOVIE_DECODE_STILL;
/* kept: this unit reads gVSyncCount as a plain int; include/clip.h and src/clip.c declare it volatile int */
extern int gVSyncCount;
short gMovieStartBlocked = 0; /* cleared in PlayMediaTrack and tested for 0 before starting a movie; never set nonzero */
short gPlayerSelection = PLAYER_SELECT_PLAY;
short gPlayerExit = 0;
int gTrackIndex = 0;
unsigned char gTrackXaFile = 0;
short gXaReading = 0;
unsigned char gPlayerInitialized = 0;
short gMovieWidth = 320;
short gMovieHeight = 240;
short gUnusedMediaShort1 = 0; /* 2 bytes; nothing reads it */
short gUnusedMediaShort2 = 0; /* 2 bytes; nothing reads it */
enum ImageChangeStep {
    IMAGE_CHANGE_SQUASH,
    IMAGE_CHANGE_SWITCH,
    IMAGE_CHANGE_GROW
};
unsigned char gImageChangeStep = IMAGE_CHANGE_SQUASH;
u_char gUnusedEndRollFlag = 0; /* 1 byte; cleared in PlayEndRoll, nothing reads it */
u_char gNextTrackImage = 1;
unsigned char gMovieStartPending = 0;
u_char gTrackImageScaleDisabled = 0; /* 1 byte; cleared in PlayIdleAudio and tested for 0 in UpdateTrackImage; never set nonzero */
u_short gIdleTrackBase[3] = { 169, 699, 0 };
u_char gFocusedWord = 0;
u_char gUnusedMediaByte = 0; /* 1 byte; nothing reads it */
unsigned char gChosenWord = 0;
unsigned char gEndRollState = END_ROLL_NONE;
unsigned char gPlayerResult = MEDIA_RESULT_NONE;
short gDecodedDataFlag = 5;
u_char gNextImageProgress = 0;
short gUnusedMediaShort3 = 0; /* 2 bytes; nothing reads it */
short gSliceBufferSize = 0x1e00;
short gDecodeMode = 2;
short gStreamMode = 0;
u_short gSliceWidth = 0x10;
FilePart *gSiteParts[2];
unsigned char gTrackProgress;
int gProgressStep;
unsigned char gEndRollStillShown;
short gTrackImageParts[3];

int RunMediaPlayer(int track, short *endRollOrWord, short *imageParts)
{
    RECT rect;

    InitMediaPlayerCube();
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    gSiteParts[0] = gSiteAParts;
    gSiteParts[1] = gSiteBParts;
    gMovieStartPending = 0;
    gEndRollStillShown = 0;
    gPlayerResult = MEDIA_RESULT_NONE;
    gImageChangeStep = IMAGE_CHANGE_SQUASH;
    if (*endRollOrWord != 0) {
        gEndRollState = END_ROLL_READY;
    } else {
        gEndRollState = END_ROLL_NONE;
    }
    gTrackImageParts[0] = imageParts[0];
    gTrackImageParts[1] = imageParts[1];
    gTrackIndex = track;
    gTrackImageParts[2] = imageParts[2];
    MuteCdAudio();
    gTrackProgress = 0;
    gProgressStep = 1;
    gActiveBuffer = GsGetActiveBuff();
    gPlayerExit = 0;
    gPlayerInitialized = 0;
    gPlayerSelection = PLAYER_SELECT_PLAY;
    gChosenWord = 0;
    *endRollOrWord = 0;
    gTrackXaFile = gTracks[gTrackIndex].xaFile;
    if (gTrackIndex >= 518) {
        RunVoiceScreen();
        return MEDIA_RESULT_COMPLETE;
    }
    SetTemporaryCaption(52, 175, "Play", 1);
    while (gPlayerExit == 0) {
        PlayMediaTrack();
        *endRollOrWord = gChosenWord;
        gPlayerInitialized = 1;
    }
    if (gEndRollState == END_ROLL_PENDING) {
        if (gTrackXaFile == 0) {
            gNodes[track].flags |= 0x4000000;
        }
        PlayEndRoll();
        gTrackIndex = 733;
        RunVoiceScreen();
        return MEDIA_RESULT_END_ROLL;
    }
    SetTemporaryCaption(0, 0, 0, 0);
    gPadEvent = PAD_EVENT_NONE;
    VSync(2);
    return gPlayerResult;
}

short gImageCornerFrame = 0;
short gWordRingPos[3] = { -1, 9, 19 };
short gFocusRingPos = 1;
u_char gUnusedTrackPlayFlag = 0; /* 1 byte; zeroed in PlayMediaTrack, nothing reads it */
int gWordSpriteWidth = 0;
short gSortPri = 19;
u_char gWordDrawOrder[3] = { 0, 1, 2 };

u_long *gStreamRing;
int gMovieBitstreamHead; /* the first word of a frame's bitstream; written in ReadMovieFrame and StartMovie, nothing reads it */
short gUnpackVSyncs;

typedef struct {
    GsSPRITE cell[7];
    u_char count;
} MeterRow;

static GsSPRITE sWordSprites[3];
static GsSPRITE sNameSprite;
static MeterRow *sMeterRows[28];
static GsSPRITE sProgressBarFrameSprite;
static GsSPRITE sNamePlateSprite;
static GsSPRITE sWordPlates[3];
static MovieDecoder sMovieDecoder;
static CdlFILE sMovieFile;
static short sTrackImageSizes[4][2];
static GsIMAGE sTimInfo;
GsSPRITE gProgressSprite;
GsSPRITE gImageCornerSprite;

int gIndex;
int gSubIndex;
int gScratch;
int gMeterCellClutY;
u_char gTrackCancelled; /* 1 byte; set to 1 on user exit in PlayMediaTrack and PlayIdleAudio, nothing reads it */
void *gDecodedAudio;
u_short gPlayerTPage;
short gTrackDone;
int gSpuIrqAddr;

static __inline__ void LoadMediaPlayerHudTimInline(int unused, short x, short y, short clutY)
{
    RECT rect;
    void *buffer;
    int request;
    u_long *image;
    GsIMAGE *tim;

    buffer = AllocHeapBlock(gBinParts[21].size);
    request = RequestFilePartRead(3, 21, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[21].size);
    FreeHeapBlock(buffer);
    image++;
    tim = &sTimInfo;
    GsGetTimInfo(image, tim);
    setRECT(&rect, x, y, tim->pw, tim->ph);
    LoadImage(&rect, tim->pixel);
    DrawSync(0);
    if ((sTimInfo.pmode >> 3) & 1) {
        setRECT(&rect, 448, clutY + 71, tim->cw, tim->ch);
        LoadImage(&rect, tim->clut);
        DrawSync(0);
    }
    FreeHeapBlock(image - 1);
}

int PlayMediaTrack(void)
{
    short buf;
    short saved;
    int vsync;
    Track *track;
    RECT rect;

    saved = 0;
    buf = gActiveBuffer;
    gFocusLines[0].y1 = 70;
    gFocusLines[0].y0 = 70;
    gFocusLines[1].x1 = 103;
    gFocusLines[1].x0 = 103;
    if (gPlayerInitialized == 0) {
        LoadMediaPlayerHudTimInline(21, 448, 0, 1);
        gPlayerTPage = GetTPage(0, 0, 448, 0);
        InitSprite(&sProgressBarFrameSprite, 161, 16, 160, 16, gPlayerTPage, 56, 0, 448, 74, 0x58000020, 0, 0);
        InitSprite(&gProgressSprite, 168, 22, 8, 8, gPlayerTPage, 176, 40, 448, 72, 0, 0, 0);
        InitSprite(&sNamePlateSprite, 240, 32, 80, 16, gPlayerTPage, 136, 16, 448, 74, 0, 0, 0);
        InitSprite(&sWordPlates[0], 40, 63, 136, 16, gPlayerTPage, 0, 16, 448, 72, 0x48000020, 0, 0);
        InitSprite(&sWordPlates[1], 133, 154, 136, 16, gPlayerTPage, 0, 16, 448, 72, 0x48000020, 0, 0);
        InitSprite(&sWordPlates[2], 160, 191, 136, 16, gPlayerTPage, 0, 16, 448, 72, 0x48000020, 0, 0);
        gWordSpriteWidth = 128;
        setRECT(&rect, 320, 72, 32, 64);
        MoveImage(&rect, 448, 100);
        DrawSync(0);
        gPlayerTPage = GetTPage(0, 0, 448, 0);
        InitSprite(&sWordSprites[0], 46, 66, gWordSpriteWidth, 16, gPlayerTPage, 0, 100, 16, 481, 0, 0, 0);
        InitSprite(&sWordSprites[1], 139, 157, gWordSpriteWidth, 16, gPlayerTPage, 0, 116, 16, 481, 0, 0, 0);
        InitSprite(&sWordSprites[2], 166, 194, gWordSpriteWidth, 16, gPlayerTPage, 0, 132, 16, 481, 0, 0, 0);
        InitSprite(&sNameSprite, 250, 36, 48, 16, gPlayerTPage, 0, 148, 0, 481, 0, 0, 0);
    }
    gTrackDone = 0;
    gTrackCancelled = 0;
    if (gTrackXaFile == 0) {
        Track *tracks = gTracks;
        track = &tracks[gTrackIndex];
        gXaReading = 0;
        gMovieStartBlocked = 0;
        gMovieState = MOVIE_DECODE_STILL;
        gProgressStep = track->length / 18;
        if (gProgressStep <= 0) {
            gProgressStep = 1;
        }
        StartMovie(gDiscFiles[track->source].sector);
    } else {
        gNextTrackImage = 1;
        gDecodedAudio = AllocHeapBlock(0x1000);
        gFocusedWord = 0;
        gDecodedDataFlag = 5;
        SpuSetTransferCallback(SwapSpuIrqAddr);
        SpuSetIRQCallback(ReadDecodedAudio);
        SpuSetIRQAddr(gSpuIrqAddr = 0x100);
        gFocusRingPos = 1;
        gWordRingPos[0] = -1;
        gWordRingPos[1] = 9;
        gMovieState = MOVIE_STILL;
        gMovieStartBlocked = 0;
        gWordRingPos[2] = 19;
        LoadTrackImages();
        for (gIndex = 0; gIndex < 15; gIndex++) {
            sMeterRows[gIndex] = (MeterRow *)AllocHeapBlock(0x100);
            sMeterRows[gIndex]->count = 0;
            for (gSubIndex = 0; gSubIndex < 7; gSubIndex++) {
                InitSprite(&sMeterRows[gIndex]->cell[gSubIndex], gSubIndex << 4, gIndex << 4, 16, 16,
                              GetTPage(0, 0, 448, 0), 0, 0, 448, 74, 0x48000020, 0, 0);
            }
        }
        gNextImageProgress = 6;
        if (gTrackImageParts[2] == -1) {
            gNextImageProgress = 9;
        }
        SeekXaFile();
    }
    gMovieState = MOVIE_DECODE_STILL;
    for (;;) {
        if (gTrackXaFile == 0) {
            if (gMovieState != MOVIE_STILL) {
                DecodeNextMovieFrame();
                if (gMovieState < MOVIE_STILL && ++gMovieState == MOVIE_STILL) {
                    CdControlF(CdlPause, 0);
                    setRECT(&rect, 320, 240, 320, 240);
                    MoveImage(&rect, 0, buf * 240);
                    DrawSync(0);
                }
            } else {
                setRECT(&rect, 320, 240, 320, 240);
                MoveImage(&rect, 0, buf * 240);
                DrawSync(0);
            }
        } else {
            setRECT(&rect, 0, buf * 240, 320, 240);
            ClearImage(&rect, 0, 0, 0);
            DrawSync(0);
        }
        GsSetRefView2(&gView);
        GsSetWorkBase(gPacketArea[buf]);
        GsClearOt(0, 0, &gOt[buf]);
        GsClearOt(0, 0, &gSubOt[buf]);
        DrawSync(0);
        gActiveBuffer = GsGetActiveBuff();
        UpdateCaption(0);
        gUnusedTrackPlayFlag = 0;
        gProgressSprite.v = 40;
        gProgressSprite.u = 176;
        if (sTrackImageSizes[gNextTrackImage][0] != -1) {
            UpdateTrackImage();
        }
        gSortPri = 19;
        if (gXaReading != 0 || gMovieState > MOVIE_STILL) {
            if (gTrackProgress < 19) {
                gProgressSprite.x = gTrackProgress * 8 + 170;
                if (gTrackProgress >= 6) {
                    gProgressSprite.u = 136;
                    gProgressSprite.w = 48;
                    gProgressSprite.x -= gProgressSprite.w;
                } else {
                    gProgressSprite.u = 176 - gTrackProgress * 8;
                    gProgressSprite.w = gTrackProgress * 8;
                    gProgressSprite.x -= gProgressSprite.w;
                }
            } else {
                gProgressSprite.u = 136;
                gProgressSprite.w = 48;
            }
            GsSortFastSprite(&gProgressSprite, &gSubOt[buf], gSortPri++);
        }
        GsSortSprite(&sProgressBarFrameSprite, &gSubOt[buf], gSortPri++);
        GsSortFastSprite(&sNameSprite, &gSubOt[buf], gSortPri++);
        GsSortFastSprite(&sNamePlateSprite, &gSubOt[buf], gSortPri++);
        if (gTrackXaFile != 0) {
            gScratch = 0;
            if (gXaReading != 0) {
                sMeterRows[7]->count = GetCdPeakLevel();
                if (sMeterRows[7]->count >= 8) {
                    sMeterRows[7]->count = 7;
                }
                for (gIndex = 0; gIndex < 7; gIndex++) {
                    if (sMeterRows[7]->count - gIndex > 0) {
                        sMeterRows[6 - gIndex]->count = sMeterRows[7]->count - gIndex;
                        sMeterRows[gIndex + 8]->count = sMeterRows[7]->count - gIndex;
                    } else if (sMeterRows[7]->count != 0) {
                        sMeterRows[6 - gIndex]->count = rand() / 0x1fff;
                        sMeterRows[gIndex + 8]->count = rand() / 0x1fff;
                    } else {
                        sMeterRows[6 - gIndex]->count = rand() / 0x3fff;
                        sMeterRows[gIndex + 8]->count = rand() / 0x3fff;
                    }
                }
                for (gIndex = 0; gIndex < 15; gIndex++) {
                    gScratch = sMeterRows[gIndex]->count / 2;
                    for (gSubIndex = 0; gSubIndex < sMeterRows[gIndex]->count; gSubIndex++) {
                        if (gScratch != 0) {
                            if (gSubIndex >= gScratch) {
                                gMeterCellClutY = 73;
                            } else {
                                gMeterCellClutY = 72;
                            }
                        } else {
                            gMeterCellClutY = 72;
                        }
                        sMeterRows[gIndex]->cell[gSubIndex].cy = gMeterCellClutY;
                        GsSortFastSprite(&sMeterRows[gIndex]->cell[gSubIndex], &gSubOt[buf], gSortPri++);
                    }
                }
            }
            for (gIndex = 0; gIndex < 2; gIndex++) {
                for (gSubIndex = gIndex + 1; gSubIndex < 3; gSubIndex++) {
                    if (sWordPlates[gWordDrawOrder[gSubIndex]].x < sWordPlates[gWordDrawOrder[gIndex]].x) {
                        gScratch = gWordDrawOrder[gSubIndex];
                        gWordDrawOrder[gSubIndex] = gWordDrawOrder[gIndex];
                        gWordDrawOrder[gIndex] = gScratch;
                    }
                }
            }
            if (gEndRollState == END_ROLL_NONE) {
                for (gIndex = 0; gIndex < 3; gIndex++) {
                    GsSortFastSprite(&sWordSprites[gWordDrawOrder[gIndex]], &gSubOt[buf], gSortPri++);
                    GsSortFastSprite(&sWordPlates[gWordDrawOrder[gIndex]], &gSubOt[buf], gSortPri++);
                }
                GsSortLine(&gFocusLines[0], &gSubOt[buf], gSortPri++);
                GsSortLine(&gFocusLines[1], &gSubOt[buf], gSortPri++);
            }
            GsSortFastSprite(&gImageCornerSprite, &gSubOt[buf], gSortPri++);
            if (++gImageCornerFrame >= 8) {
                gImageCornerFrame = 0;
            }
            gImageCornerSprite.u = gImageCornerFrame * 24;
            SortRotatedSpriteQuad((SpriteQuad *)gTrackImage, &gOt[buf], 350);
        }
        if (gMovieStartPending == 1 && gCaptionMoving == 0) {
            gMovieState = MOVIE_PLAYING;
            gMovieStartBlocked = 0;
            CdRead2(CdlModeStream2 | CdlModeSpeed | CdlModeRT);
            gMovieStartPending = 0;
        }
        if (gCaptionMoving == 0) {
            ReadPadEvent();
            if (gPadEvent == PAD_EVENT_CIRCLE) {
                if (gPlayerSelection == PLAYER_SELECT_PLAY) {
                    UnmuteCdAudio();
                    if (gEndRollState == END_ROLL_READY) {
                        gEndRollState = END_ROLL_PENDING;
                    }
                    if (gTrackXaFile != 0 && gXaReading == 0) {
                        PlaySound(26);
                        StartXaTrack(gTracks[gTrackIndex].source);
                    } else if (gMovieStartBlocked == 0 && gMovieState == MOVIE_STILL && gTrackXaFile == 0) {
                        PlaySound(26);
                        gCaptionTargetX = 52;
                        gCaptionTargetY = 205;
                        memcpy(gNextCaptionText, "Exit", 5);
                        UpdateCaption(1);
                        gPlayerSelection = PLAYER_SELECT_EXIT;
                        gMovieStartPending = 1;
                    }
                } else if ((gPlayerSelection == PLAYER_SELECT_EXIT || gPlayerSelection == PLAYER_SELECT_WORD) && gEndRollState < END_ROLL_PENDING) {
                    PlaySound(27);
                    gPlayerExit = 1;
                    gTrackDone = 1;
                    gTrackCancelled = 1;
                    if (gPlayerSelection == PLAYER_SELECT_WORD) {
                        gChosenWord = gFocusedWord + 1;
                    }
                }
            }
            if (gMovieState != MOVIE_PLAYING) {
                if (gPadEvent == PAD_EVENT_UP) {
                    if (gPlayerSelection <= PLAYER_SELECT_EXIT) {
                        gCaptionTargetX = 52;
                        gCaptionTargetY = 175;
                        memcpy(gNextCaptionText, "Play", 5);
                        UpdateCaption(1);
                        gPlayerSelection = PLAYER_SELECT_PLAY;
                    } else if (gPlayerSelection == PLAYER_SELECT_WORD) {
                        for (gIndex = 0; gIndex < 3; gIndex++) {
                            sWordPlates[gIndex].cy = 72;
                            sWordSprites[gIndex].cx = 16;
                        }
                        gPlayerSelection = PLAYER_SELECT_WORD_UP;
                        PlaySound(1);
                    }
                }
                if (gPadEvent == PAD_EVENT_DOWN) {
                    if (gPlayerSelection < PLAYER_SELECT_WORD && gPlayerSelection != PLAYER_SELECT_EXIT) {
                        gCaptionTargetX = 52;
                        gCaptionTargetY = 205;
                        memcpy(gNextCaptionText, "Exit", 5);
                        UpdateCaption(1);
                        gPlayerSelection = PLAYER_SELECT_EXIT;
                    } else if (gPlayerSelection == PLAYER_SELECT_WORD) {
                        for (gIndex = 0; gIndex < 3; gIndex++) {
                            sWordPlates[gIndex].cy = 72;
                            sWordSprites[gIndex].cx = 16;
                        }
                        gPlayerSelection = PLAYER_SELECT_WORD_DOWN;
                        PlaySound(1);
                    }
                }
            }
            if (gTrackXaFile != 0) {
                if (gPadEvent == PAD_EVENT_RIGHT && gEndRollState == END_ROLL_NONE) {
                    sWordPlates[gFocusedWord].cy = 73;
                    sWordSprites[gFocusedWord].cx = 32;
                    if (gPlayerSelection < PLAYER_SELECT_WORD) {
                        saved = gPlayerSelection;
                        gPlayerSelection = PLAYER_SELECT_WORD;
                    }
                }
                if (gPadEvent == PAD_EVENT_LEFT && gPlayerSelection >= PLAYER_SELECT_WORD) {
                    sWordPlates[gFocusedWord].cy = 72;
                    sWordSprites[gFocusedWord].cx = 16;
                    if (gPlayerSelection == PLAYER_SELECT_WORD) {
                        gPlayerSelection = saved;
                    }
                }
            }
        }
        if (gTrackXaFile != 0) {
            gPlayerTPage = GetTPage(0, 0, 448, 0);
            if (gPlayerSelection == PLAYER_SELECT_WORD_UP) {
                for (gIndex = 0; gIndex < 3; gIndex++) {
                    if (++gWordRingPos[gIndex] >= 30) {
                        gWordRingPos[gIndex] = 0;
                    }
                    sWordSprites[gIndex].x = gWordRing[gWordRingPos[gIndex]][0] + 6;
                    sWordSprites[gIndex].y = gWordRing[gWordRingPos[gIndex]][1] + 3;
                    InitSprite(&sWordPlates[gIndex], gWordRing[gWordRingPos[gIndex]][0],
                                  gWordRing[gWordRingPos[gIndex]][1], 136, 16, gPlayerTPage, 0, 16, 448, 72, 0, 0, 0);
                }
                if (--gFocusRingPos < 0) {
                    gFocusRingPos = 29;
                }
                gFocusLines[0].y0 = gWordRing[gFocusRingPos][1] + 7;
                gFocusLines[0].y1 = gWordRing[gFocusRingPos][1] + 7;
                gFocusLines[1].x0 = gWordRing[gFocusRingPos][0] + 63;
                gFocusLines[1].x1 = gWordRing[gFocusRingPos][0] + 63;
            }
            if (gPlayerSelection == PLAYER_SELECT_WORD_DOWN) {
                if (gWordRingPos[0] == -1) {
                    gWordRingPos[0] = 1;
                    gWordRingPos[1] = 11;
                    gFocusRingPos = -1;
                    gWordRingPos[2] = 21;
                }
                for (gIndex = 2; gIndex >= 0; gIndex--) {
                    if (--gWordRingPos[gIndex] < 0) {
                        gWordRingPos[gIndex] = 29;
                    }
                    sWordSprites[gIndex].x = gWordRing[gWordRingPos[gIndex]][0] + 6;
                    sWordSprites[gIndex].y = gWordRing[gWordRingPos[gIndex]][1] + 3;
                    InitSprite(&sWordPlates[gIndex], gWordRing[gWordRingPos[gIndex]][0],
                                  gWordRing[gWordRingPos[gIndex]][1], 136, 16, gPlayerTPage, 0, 16, 448, 72, 0, 0, 0);
                }
                if (++gFocusRingPos >= 30) {
                    gFocusRingPos = 0;
                }
                gFocusLines[0].y0 = gWordRing[gFocusRingPos][1] + 7;
                gFocusLines[0].y1 = gWordRing[gFocusRingPos][1] + 7;
                gFocusLines[1].x0 = gWordRing[gFocusRingPos][0] + 63;
                gFocusLines[1].x1 = gWordRing[gFocusRingPos][0] + 63;
            }
            if (gPlayerSelection >= PLAYER_SELECT_WORD_UP) {
                for (gIndex = 0; gIndex < 3; gIndex++) {
                    if (gFocusRingPos == gWordRingPos[gIndex]) {
                        if (gPlayerInitialized != 0) {
                            gPlayerSelection = PLAYER_SELECT_WORD;
                        }
                        gPlayerInitialized = 1;
                        sWordSprites[gIndex].cx = 32;
                        sWordPlates[gIndex].cy = 73;
                        gFocusedWord = gIndex;
                        break;
                    }
                    sWordSprites[gIndex].cx = 16;
                }
            }
            DrawTileGrid();
        }
        DrawMediaPlayerCube(gPlayerSelection);
        DrawSync(0);
        GsSortOt(&gSubOt[buf], &gOt[buf]);
        if (gTrackXaFile != 0) {
            GsSortClear(0, 0, 0, &gOt[buf]);
        }
        GsDrawOt(&gOt[buf]);
        if (DrawSync(1) != 0) {
            DrawSync(0);
        }
        vsync = 5;
        if (gMovieState != MOVIE_STILL && gTrackXaFile == 0) {
            vsync = 0;
        }
        VSync(vsync);
        GsSwapDispBuff();
        buf = GsGetActiveBuff();
        gActiveBuffer = buf;
        if (gTrackDone == 1) {
            break;
        }
    }
    gMovieStartPending = 0;
    gMovieState = MOVIE_DECODE_STILL;
    gTrackProgress = 0;
    MuteCdAudio();
    if (gEndRollState == END_ROLL_PENDING) {
        gPlayerExit = 1;
    }
    if (gTrackXaFile == 0) {
        if (gTrackDone == 1) {
            StopMovie();
        }
    } else {
        if (CdSync(1, 0) == 0) {
            CdControlF(CdlPause, 0);
        }
        gXaReading = 0;
        for (gIndex = 0; gIndex < 15; gIndex++) {
            FreeHeapBlock(sMeterRows[gIndex]);
        }
        FreeHeapBlock(gDecodedAudio);
        SpuSetIRQ(SPU_OFF);
        SpuSetIRQCallback(0);
        SpuSetTransferCallback(0);
    }
}

short gMovieFrameNumbers[2];
short gEndRollSegment;

extern u_long StCdIntrFlag;

void LoadDecodedSlice(void)
{
    MovieDecoder *dec;
    RECT snapRect;
    char unused[8]; /* kept: nothing reads it; without it the routine compiles differently */
    int imageId;

    if (gMovieState == MOVIE_STILL) {
        return;
    }
    if (gStreamMode == 1 && StCdIntrFlag != 0) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    dec = &sMovieDecoder;
    imageId = dec->sliceIndex;
    snapRect = dec->sliceRect;
    dec->sliceIndex = dec->sliceIndex ? 0 : 1;
    dec->sliceRect.x += dec->sliceRect.w;
    if (dec->sliceRect.x < dec->frameRect[dec->frameIndex].x + dec->frameRect[dec->frameIndex].w) {
        DecDCTout(dec->slice[dec->sliceIndex], dec->sliceRect.w * dec->sliceRect.h / 2);
    } else {
        u_char completed = dec->frameIndex;
        dec->frameDone = 1;
        dec->frameIndex = dec->frameIndex ? 0 : 1;
        dec->sliceRect.x = dec->frameRect[dec->frameIndex].x;
        gCompletedFrameIndex = completed;
        dec->sliceRect.y = dec->frameRect[dec->frameIndex].y;
    }
    if (gMovieState < MOVIE_STILL) {
        if (gPlayerInitialized != 0) {
            return;
        }
        snapRect.y = 240;
        snapRect.x += 320;
        LoadImage(&snapRect, sMovieDecoder.slice[imageId]);
    } else {
        snapRect.y = gActiveBuffer * 240;
        LoadImage(&snapRect, sMovieDecoder.slice[imageId]);
    }
    DrawSync(0);
}

MovieFrame *GetMovieFrame(MovieDecoder *dec)
{
    MovieFrame *frame;
    StHEADER *header;
    int timeout;

    timeout = 2000;
    while (StGetNext((u_long **)&frame, (u_long **)&header) != 0) {
        if (--timeout == 0) {
            return 0;
        }
    }
    gMovieFrameNumbers[dec->frameIndex] = frame->number;
    if (header->frameCount + 1 >= gTracks[gTrackIndex].length - 4) {
        gTrackProgress = 18;
        gPlayerResult = MEDIA_RESULT_COMPLETE;
        gTrackDone = 1;
    }
    if (gTrackDone == 0 && gMovieState > MOVIE_STILL) {
        gTrackProgress = header->frameCount / gProgressStep;
    }
    if (gEndRollState == END_ROLL_PLAYING) {
        if (gEndRollSegments[gEndRollSegment].end < gMovieFrameNumbers[dec->frameIndex]) {
            gEndRollStillShown = 0;
            if (gEndRollSegment < 54) {
                gEndRollSegment++;
            }
        }
        if (gEndRollSegments[gEndRollSegment].start <= gMovieFrameNumbers[dec->frameIndex]) {
            if (gEndRollSegments[gEndRollSegment].showStill == 1) {
                gEndRollStillShown = 1;
            }
        }
    }
    dec->frameRect[1].w = gMovieWidth;
    dec->frameRect[0].w = gMovieWidth;
    dec->frameRect[1].h = gMovieHeight;
    dec->frameRect[0].h = gMovieHeight;
    dec->sliceRect.h = gMovieHeight;
    return frame;
}

u_char gCompletedFrameIndex; /* last completed frame buffer index (0 or 1); nothing reads it */
u_long *gEndRollStill;

static inline void WaitMovieFrameDone(MovieDecoder *dec)
{
    volatile int wait; /* polls dec->frameDone, set by DecDCToutCallback (LoadDecodedSlice) */

    wait = 0x800000;
    while (dec->frameDone == 0) {
        if (--wait == 0) {
            dec->frameDone = 1;
            dec->frameIndex = dec->frameIndex == 0;
            dec->sliceRect.x = dec->frameRect[dec->frameIndex].x;
            dec->sliceRect.y = dec->frameRect[dec->frameIndex].y;
        }
    }
    dec->frameDone = 0;
}

static inline int ReadMovieFrame(MovieDecoder *dec)
{
    MovieFrame *frame;
    int timeout;

    timeout = 2000;
    while ((frame = GetMovieFrame(dec)) == 0) {
        if (--timeout == 0) {
            return -1;
        }
    }
    dec->runLevelIndex = dec->runLevelIndex == 0;
    gLumaQuant = frame->lumaQuant;
    gChromaQuant = frame->chromaQuant;
    gMovieBitstreamHead = *frame->bitstream;
    if (gEndRollStillShown == 0) {
        gUnpackVSyncs = gVSyncCount;
        UnpackNitrBsFrame(frame->bitstream, dec->runLevel[dec->runLevelIndex]);
        gUnpackVSyncs = gVSyncCount - gUnpackVSyncs;
    }
    StFreeRing((u_long *)frame);
    return 0;
}

static inline void LoadTimToRect(u_long *tim, RECT rect)
{
    GsGetTimInfo(tim, &sTimInfo);
    LoadImage(&rect, sTimInfo.pixel);
}

int StartXaTrack(int channel)
{
    CdlFILTER filter;
    CdlFILE entry;
    int start;
    int end;
    int track;

    filter.file = 1;
    filter.chan = channel;
    CdIntToPos(gDiscFiles[gXaFiles[gTrackXaFile - 1]].sector, &entry.pos);
    CdControl(CdlSetfilter, (u_char *)&filter, 0);
    start = CdPosToInt(&entry.pos);
    track = gTrackIndex;
    if (track >= 518) {
        end = start + (gTracks[track].length / 2336 * 32);
    } else {
        end = (int)((double)start + (gTracks[track].length - 5139.200000000001) / 2336.0 * 32.0);
    }
    gProgressStep = (end - start) / 18;
    if (gProgressStep <= 0) {
        gProgressStep = 1;
    }
    return StartXaRead(start, end);
}

void PollXaPlayback(void)
{
    union {
        u_char bytes[8];
        struct {
            u_char track;
            u_char index;
            u_char relativeMSF[3];
            CdlLOC absolute;
        } position;
    } result;
    int status;
    int sector;
    u_char progress;

    memset(result.bytes, 0, sizeof(result.bytes));
    if (gXaReading == 0 || (VSync(-1) & 3) != 0) {
        return;
    }
    status = CdSync(1, result.bytes);
    if (status == CdlDiskError) {
        RestartXaRead(CdlReadS);
        return;
    }
    if (status != CdlComplete) {
        return;
    }
    if (gXaSector > gXaEndSector || gXaSector < gXaStartSector - 300) {
        SpuSetIRQ(SPU_OFF);
        MuteCdAudio();
        CdControlB(CdlPause, 0, 0);
        gXaReading = 0;
        gTrackProgress = 0;
        gNextTrackImage = 1;
        if (gEndRollState != status) {
            SeekXaFile();
            ShowTrackImage(0);
        }
        gNextImageProgress = 6;
        if (gTrackImageParts[2] == -1) {
            gNextImageProgress = 9;
        }
        gPlayerResult = status;
        if (gEndRollState == status) {
            gTrackDone = 1;
        }
        return;
    }
    if (CdLastCom() == CdlGetlocP) {
        sector = CdPosToInt(&result.position.absolute);
        if (sector > 0) {
            gXaSector = sector;
        }
    }
    progress = (gXaSector - gXaStartSector) / gProgressStep;
    if (progress > gTrackProgress) {
        gTrackProgress = progress;
    }
    CdControlF(CdlGetlocP, 0);
}

void DecodeNextMovieFrame()
{
    RECT rect;

    if (gEndRollStillShown == 0) {
        DecDCTin(sMovieDecoder.runLevel[sMovieDecoder.runLevelIndex], gDecodeMode);
        DecDCTout(sMovieDecoder.slice[sMovieDecoder.sliceIndex], sMovieDecoder.sliceRect.w * sMovieDecoder.sliceRect.h / 2);
    } else {
        setRECT(&rect, 0, gActiveBuffer * 240, 480, 240);
        LoadTimToRect(gEndRollStill + 4, rect);
        DrawSync(0);
        gCompletedFrameIndex = sMovieDecoder.frameIndex;
        sMovieDecoder.frameDone = 1;
        sMovieDecoder.frameIndex = sMovieDecoder.frameIndex == 0;
    }
    ReadMovieFrame(&sMovieDecoder);
    WaitMovieFrameDone(&sMovieDecoder);
}

void StartMovie(int sector)
{
    MovieDecoder *dec = &sMovieDecoder;
    CdlLOC *loc = &sMovieFile.pos;
    void (*callback)(void);

    CdIntToPos(sector, loc);
    dec->runLevel[0] = (u_long *)AllocHeapBlock(0x28000);
    dec->runLevel[1] = (u_long *)AllocHeapBlock(0x28000);
    dec->runLevelIndex = 0;
    dec->slice[0] = (u_long *)AllocHeapBlock(gSliceBufferSize);
    dec->slice[1] = (u_long *)AllocHeapBlock(gSliceBufferSize);
    dec->sliceIndex = 0;
    setRECT(&dec->frameRect[0], 0, 0, 0, 0);
    setRECT(&dec->frameRect[0], 0, 240, 0, 0);
    dec->frameIndex = 0;
    dec->sliceRect.x = 0;
    dec->sliceRect.y = 0;
    dec->sliceRect.w = gSliceWidth;
    dec->sliceRect.h = 0;
    dec->frameDone = 0;
    callback = LoadDecodedSlice;
    DecDCTReset(0);
    DecDCToutCallback(callback);
    gStreamRing = (u_long *)AllocHeapBlock(0x10000);
    StSetRing(gStreamRing, 32);
    StSetStream(gStreamMode, 1, -1, 0, 0);
    StartStreamRead((u_char *)loc);
    BuildSignedRunLevelCodes(gNitrBsRunLevelCodes);
    while (ReadMovieFrame(&sMovieDecoder) == -1) {
        gMovieSeekLoc = sMovieFile.pos;
        do {
            while (CdControlB(CdlSeekP, (u_char *)&gMovieSeekLoc, 0) == 0) {
            }
            CdControl(CdlSetloc, (u_char *)&gMovieSeekLoc, 0);
        } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT | CdlModeSize1) == 0);
    }
}

CdlLOC gMovieSeekLoc;

void PlayIdleMovie(void)
{
    RECT rect;
    SpuCommonAttr attr;
    SpuCommonAttr *p;

    gTrackDone = 0;
    gEndRollState = END_ROLL_NONE;
    gEndRollStillShown = 0;
    if (gP2NodesOpened != 63) {
        if (gCurrentDisc != 0) {
            gTrackIndex = rand() / 4681 + 724;
        } else {
            gTrackIndex = rand() / 1489 + 709;
        }
    } else if (gCurrentDisc == 1) {
        gTrackIndex = rand() / 4681 + 724;
        if (rand() / 10922 == 1) {
            gTrackIndex = rand() % 2 + 731;
        }
    } else {
        gTrackIndex = rand() / 1489 + 709;
    }
    if (gTrackIndex > gCurrentDisc * 2 + 730) {
        gTrackIndex = gCurrentDisc * 2 + 730;
    }
    srand(rand());
    gActiveBuffer = GsGetActiveBuff();
    p = &attr;
    gTrackProgress = 0;
    gProgressStep = 1;
    p->mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
              SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    gMovieState = MOVIE_PLAYING;
    p->mvol.left = 0x3fff;
    p->mvol.right = 0x3fff;
    p->mvolmode.left = 0x3fff;
    p->mvolmode.right = 0x3fff;
    p->cd.volume.left = 0x3fff;
    p->cd.volume.right = 0x3fff;
    p->cd.mix = SPU_ON;
    SpuSetCommonAttr(p);
    StartMovie(gDiscFiles[gTracks[gTrackIndex].source].sector);
    do {
        DecodeNextMovieFrame();
        VSync(0);
        DrawSync(0);
        GsSwapDispBuff();
        gActiveBuffer = GsGetActiveBuff();
        ReadPadEvent();
        if (gPadEvent == PAD_EVENT_CIRCLE) {
            gTrackDone = 1;
        }
    } while (gTrackDone != 1);
    StopMovie();
    p = &attr;
    p->mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
              SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    p->mvol.left = 0x4fff;
    p->mvol.right = 0x4fff;
    p->mvolmode.left = 0x4fff;
    p->mvolmode.right = 0x4fff;
    attr.cd.volume.left = 0;
    attr.cd.volume.right = 0;
    p->cd.mix = SPU_ON;
    SpuSetCommonAttr(p);
    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
}

static inline void SetCommonVolume(short master, short cd)
{
    SpuCommonAttr attr;

    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
                SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    attr.mvol.left = master;
    attr.mvol.right = master;
    attr.mvolmode.left = master;
    attr.mvolmode.right = master;
    attr.cd.volume.left = cd;
    attr.cd.volume.right = cd;
    attr.cd.mix = SPU_ON;
    SpuSetCommonAttr(&attr);
}

static inline void SeekCurrentXaFile(void)
{
    CdlLOC loc;

    CdIntToPos(gDiscFiles[gXaFiles[gTrackXaFile - 1]].sector, &loc);
    CdControl(CdlSetloc, (u_char *)&loc, 0);
    CdControl(CdlSeekP, (u_char *)&loc, 0);
}

static inline void LoadEndRollStillPart(int part, void *dest)
{
    u_short group[2] = { 6, 6 };
    int request;

    request = RequestFilePartRead(group[1], part, gSiteParts[1], dest);
    while (PollCdRequest(request) == 0) {
    }
}

void PlayIdleAudio(void)
{
    RECT rect; /* 8 bytes; nothing uses it */
    Track *track;

    gTrackDone = 0;
    gEndRollState = END_ROLL_PENDING;
    gXaReading = 0;
    gTrackIndex = gIdleTrackBase[gCurrentDisc] + rand() / 3640;
    srand(rand());
    gNextImageProgress = 6;
    gNextTrackImage = 1;
    gTrackXaFile = gTracks[gTrackIndex].xaFile;
    gActiveBuffer = GsGetActiveBuff();
    gTrackImageScaleDisabled = 0;
    gTrackProgress = 0;
    gProgressStep = 1;
    SetCommonVolume(0x3fff, 0x3fff);
    gTrackImageParts[0] = gIdleTrackImageParts[gCurrentDisc][gTrackIndex - gIdleTrackBase[gCurrentDisc]][0];
    gTrackImageParts[1] = gIdleTrackImageParts[gCurrentDisc][gTrackIndex - gIdleTrackBase[gCurrentDisc]][1];
    gTrackImageParts[2] = gIdleTrackImageParts[gCurrentDisc][gTrackIndex - gIdleTrackBase[gCurrentDisc]][2];
    LoadTrackImages();
    SeekCurrentXaFile();
    track = &gTracks[gTrackIndex];
    StartXaTrack(track->source);
    gTrackCancelled = 0;
    for (;;) {
        GsSetRefView2(&gView);
        GsSetWorkBase(gPacketArea[gActiveBuffer]);
        GsClearOt(0, 0, &gOt[gActiveBuffer]);
        UpdateTrackImage();
        DrawTileGrid();
        SortRotatedSpriteQuad((SpriteQuad *)gTrackImage, &gOt[gActiveBuffer], 350);
        GsSortClear(0, 0, 0, &gOt[gActiveBuffer]);
        GsDrawOt(&gOt[gActiveBuffer]);
        VSync(5);
        GsSwapDispBuff();
        gActiveBuffer = GsGetActiveBuff();
        ReadPadEvent();
        if (gPadEvent == PAD_EVENT_CIRCLE) {
            gTrackCancelled = 1;
            gTrackDone = 1;
        }
        if (gTrackDone == 1) {
            gXaReading = 0;
            gEndRollState = END_ROLL_NONE;
            break;
        }
    }
    SetCommonVolume(0x4fff, 0);
}

int gUnusedEndRollWord; /* 4 bytes; zeroed in PlayEndRoll, nothing reads it */
void *gPackedImageBuffer;

int PlayEndRoll(void)
{
    RECT rect;
    int i;
    int movieCount;
    int playbackValue;
    int flag;

    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 480;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    GsInitGraph2(320, 240, 4, 0, 1);
    gUnusedEndRollFlag = 0;
    gCompletedFrameIndex = 0;
    movieCount = 0;
    for (i = 0; i < 716; i++) {
        if ((((u_int)gNodes[i].flags >> 20) & 0xf) == NODE_KIND_DC) {
            flag = ((u_int)gNodes[i].flags >> 26) & 1;
            if (flag == 1) {
                gEndRollSegments[movieCount].showStill = 0;
            } else {
                gEndRollSegments[movieCount].showStill = 1;
            }
            movieCount++;
        }
    }
    playbackValue = 3;
    gDecodeMode = playbackValue;
    gEndRollState = END_ROLL_PLAYING;
    gStreamMode = 1;
    gSliceBufferSize = 0x2d00;
    gSliceWidth = 24;
    gEndRollStillShown = 0;
    gTrackDone = 0;
    gTrackIndex = 735;
    playbackValue = gMovieWidth;
    gMovieWidth = playbackValue * 3 / 2;
    gActiveBuffer = GsGetActiveBuff();
    gTrackProgress = 0;
    gProgressStep = 1;
    gMovieState = MOVIE_PLAYING;
    gEndRollSegment = 0;
    gXaReading = 0;
    SetCommonVolume(0x3fff, 0x3fff);
    gUnusedEndRollWord = 0;
    while (gActiveBuffer != 0) {
        GsSwapDispBuff();
        gActiveBuffer = GsGetActiveBuff();
    }
    gPackedImageBuffer = AllocHeapBlock(gSiteParts[1][554].size);
    LoadEndRollStillPart(554, gPackedImageBuffer);
    gEndRollStill = (u_long *)UnpackNapk(gPackedImageBuffer, gSiteParts[1][554].size);
    FreeHeapBlock(gPackedImageBuffer);
    StartMovie(gDiscFiles[gTracks[gTrackIndex].source].sector);
    do {
        DecodeNextMovieFrame();
        DrawSync(0);
        VSync(0);
        GsSwapDispBuff();
        gActiveBuffer = GsGetActiveBuff();
        gUnusedEndRollFlag = 0;
    } while (gTrackDone != 1);
    FreeHeapBlock(gEndRollStill);
    gMovieState = MOVIE_PLAYING;
    DecDCToutCallback(0);
    StUnSetRing();
    if (gMovieState == MOVIE_PLAYING) {
        if (CdSync(1, 0) == 0) {
            CdControlF(CdlPause, 0);
        }
        gMovieState = MOVIE_DECODE_STILL;
    }
    FreeHeapBlock(gStreamRing);
    FreeHeapBlock(sMovieDecoder.runLevel[0]);
    FreeHeapBlock(sMovieDecoder.runLevel[1]);
    FreeHeapBlock(sMovieDecoder.slice[0]);
    FreeHeapBlock(sMovieDecoder.slice[1]);
    gMovieWidth = 320;
    gDecodeMode = 2;
    gSliceBufferSize = 0x1e00;
    gEndRollSegment = 0;
    gStreamMode = 0;
    gSliceWidth = 16;
    GsInitGraph2(320, 240, 4, 1, 0);
    DrawSync(0);
    VSync(0);
    GsSwapDispBuff();
    rect.w = 640;
    rect.x = 0;
    rect.y = 0;
    rect.h = 480;
    ClearImage(&rect, 0, 0, 0);
    return DrawSync(0);
}

void ShowTrackImage(int frame)
{
    SpriteQuad *quad;
    short *size;
    short index;
    int paletteY;
    int originalFrame;
    int paletteX;
    short (*sizes)[2];
    SpriteQuad *output;
    GsSPRITE *corner;
    short *finalSize;
    short (*finalSizes)[2];
    short height;
    short cornerX, cornerY;
    short x, y;

    quad = (SpriteQuad *)gTrackImage;
    paletteX = 320;
    index = frame;
    paletteY = index + 201;
    sizes = sTrackImageSizes;
    size = sizes[index];
    x = (paletteX - size[0]) / 2;
    quad->x = x;
    y = (240 - size[1]) / 2;
    quad->y = y;
    originalFrame = frame;
    quad->clut = GetClut(paletteX, paletteY);
    quad->v = 0;
    if (index == 0) {
        quad->tpage = GetTPage(1, 0, 320, 0);
        quad->u = 0;
    } else if (index == 2) {
        quad->tpage = GetTPage(1, 0, 448, 256);
        quad->u = 0;
    } else {
        quad->tpage = GetTPage(1, 0, 320, 256);
        quad->u = 0;
    }
    output = (SpriteQuad *)gTrackImage;
    finalSizes = sTrackImageSizes;
    finalSize = finalSizes[(short)originalFrame];
    corner = &gImageCornerSprite;
    output->w = finalSize[0];
    height = finalSize[1];
    cornerX = output->x + 5;
    cornerY = output->y + 5;
    corner->x = cornerX;
    corner->y = cornerY;
    output->h = height;
}

static inline void LoadSiteFilePart(int part, void *dest)
{
    u_short group[2] = { 6, 6 };
    int request;

    request = RequestFilePartRead(group[gCurrentDisc], part, gSiteParts[gCurrentDisc], dest);
    while (PollCdRequest(request) == 0) {
        DrawLoadingFrame(0);
    }
}

static inline void UploadTrackTim(u_long *tim, int x, int y, int slot)
{
    RECT rect;

    GsGetTimInfo(tim, &sTimInfo);
    setRECT(&rect, x, y, sTimInfo.pw, sTimInfo.ph);
    LoadImage(&rect, sTimInfo.pixel);
    DrawSync(0);
    rect.x = x + 3;
    rect.y = y + 3;
    rect.w = sTimInfo.pw - 3;
    rect.h = sTimInfo.ph - 3;
    MoveImage(&rect, x, y);
    DrawSync(0);
    if ((sTimInfo.pmode >> 3) & 1) {
        setRECT(&rect, 320, 201 + slot, sTimInfo.cw, sTimInfo.ch);
        LoadImage(&rect, sTimInfo.clut);
        DrawSync(0);
    }
}

int LoadTrackImages(void)
{
    int i;
    int missingWidth;
    u_long *image;
    int x;
    int y;
    u_short tpage;
    u_short clut;
    int cornerX;
    int cornerY;

    missingWidth = -1;
    for (i = 2; i >= 0; i--) {
        sTrackImageSizes[i][0] = missingWidth;
    }

    gPackedImageBuffer = AllocHeapBlock(gSiteParts[gCurrentDisc][gTrackImageParts[0]].size);
    LoadSiteFilePart(gTrackImageParts[0], gPackedImageBuffer);
    image = (u_long *)UnpackNapk(gPackedImageBuffer, gSiteParts[gCurrentDisc][gTrackImageParts[0]].size);
    image++;
    UploadTrackTim(image, 320, 0, 0);
    FreeHeapBlock(image - 1);
    FreeHeapBlock(gPackedImageBuffer);
    sTrackImageSizes[0][0] = sTimInfo.pw * 2 - 17;
    x = (320 - sTrackImageSizes[0][0]) / 2;
    sTrackImageSizes[0][1] = sTimInfo.ph - 3;
    y = (240 - sTrackImageSizes[0][1]) / 2;
    tpage = GetTPage(1, 0, 320, 0);
    clut = GetClut(320, 201);
    ((SpriteQuad *)gTrackImage)->x = x;
    ((SpriteQuad *)gTrackImage)->y = y;
    ((SpriteQuad *)gTrackImage)->tpage = tpage;
    ((SpriteQuad *)gTrackImage)->u = 0;
    ((SpriteQuad *)gTrackImage)->v = 0;
    ((SpriteQuad *)gTrackImage)->clut = clut;
    ((SpriteQuad *)gTrackImage)->r = 128;
    ((SpriteQuad *)gTrackImage)->g = 128;
    ((SpriteQuad *)gTrackImage)->b = 128;
    ((SpriteQuad *)gTrackImage)->attribute = 0x1000040;
    ((SpriteQuad *)gTrackImage)->scale[0] = 4096;
    ((SpriteQuad *)gTrackImage)->scale[1] = 4096;
    ((SpriteQuad *)gTrackImage)->w = sTrackImageSizes[0][0];
    ((SpriteQuad *)gTrackImage)->h = sTrackImageSizes[0][1];
    cornerX = (320 - sTrackImageSizes[0][0]) / 2 + 5;
    cornerY = (240 - sTrackImageSizes[0][1]) / 2 + 5;
    gImageCornerSprite.tpage = GetTPage(0, 0, 448, 0);
    gImageCornerSprite.x = cornerX;
    gImageCornerSprite.y = cornerY;
    gImageCornerSprite.w = 24;
    gImageCornerSprite.v = 48;
    gImageCornerSprite.cx = 448;
    gImageCornerSprite.h = 24;
    gImageCornerSprite.u = 0;
    gImageCornerSprite.cy = 75;
    gImageCornerSprite.r = 128;
    gImageCornerSprite.g = 128;
    gImageCornerSprite.b = 128;
    gImageCornerSprite.mx = 0;
    gImageCornerSprite.my = 0;
    gImageCornerSprite.attribute = 0;

    if (gTrackImageParts[1] != -1) {
        u_long *nextImage;

        gPackedImageBuffer = AllocHeapBlock(gSiteParts[gCurrentDisc][gTrackImageParts[1]].size);
        LoadSiteFilePart(gTrackImageParts[1], gPackedImageBuffer);
        nextImage = (u_long *)UnpackNapk(gPackedImageBuffer, gSiteParts[gCurrentDisc][gTrackImageParts[1]].size);
        nextImage++;
        UploadTrackTim(nextImage, 320, 256, 1);
        FreeHeapBlock(nextImage - 1);
        sTrackImageSizes[1][0] = sTimInfo.pw * 2 - 17;
        sTrackImageSizes[1][1] = sTimInfo.ph - 3;
        FreeHeapBlock(gPackedImageBuffer);
    }
    if (gTrackImageParts[2] != -1) {
        u_long *nextImage;

        gPackedImageBuffer = AllocHeapBlock(gSiteParts[gCurrentDisc][gTrackImageParts[2]].size);
        LoadSiteFilePart(gTrackImageParts[2], gPackedImageBuffer);
        nextImage = (u_long *)UnpackNapk(gPackedImageBuffer, gSiteParts[gCurrentDisc][gTrackImageParts[2]].size);
        nextImage++;
        UploadTrackTim(nextImage, 448, 256, 2);
        FreeHeapBlock(nextImage - 1);
        FreeHeapBlock(gPackedImageBuffer);
        sTrackImageSizes[2][0] = sTimInfo.pw * 2 - 17;
        sTrackImageSizes[2][1] = sTimInfo.ph - 3;
    }
}

void UpdateTrackImage(void)
{
    SpriteQuad *quad;
    u_short scale;
    u_char count, step;

    if (gTrackProgress >= gNextImageProgress) {
        switch (gImageChangeStep) {
        case IMAGE_CHANGE_SQUASH:
            quad = (SpriteQuad *)gTrackImage;
            scale = quad->scale[1];
            if (scale != 0 && gTrackImageScaleDisabled == 0) {
                quad->scale[1] = scale - 1024;
                break;
            }
            gImageChangeStep++;
            break;
        case IMAGE_CHANGE_SWITCH:
            if (gTrackXaFile == 0) {
                break;
            }
            if ((u_char)((u_int)gNextImageProgress / 3) == 4) {
                ShowTrackImage(2);
            } else {
                ShowTrackImage(1);
            }
            gImageChangeStep++;
            break;
        case IMAGE_CHANGE_GROW:
            quad = (SpriteQuad *)gTrackImage;
            scale = quad->scale[1];
            if (scale < 4096 && gTrackImageScaleDisabled == 0) {
                quad->scale[1] = scale + 1024;
            } else {
                count = gNextImageProgress;
                step = gNextTrackImage;
                gImageChangeStep = IMAGE_CHANGE_SQUASH;
                gNextImageProgress = count << 1;
                gNextTrackImage = step + 1;
            }
            break;
        }
    }
}

int StartStreamRead(u_char *location)
{
    int started;

    do {
        while (CdControlB(CdlSeekP, location, 0) == 0) {
        }
        CdControl(CdlSetloc, location, 0);
    } while ((started = CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT | CdlModeSize1)) == 0);
    return started;
}

void ReadDecodedAudio(void)
{
    SpuSetIRQ(SPU_OFF);
    SpuReadDecodedData((SpuDecodedData *)gDecodedAudio, gDecodedDataFlag);
}

void SwapSpuIrqAddr(void)
{
    if (gSpuIrqAddr == 0) {
        gSpuIrqAddr = 0x200;
    } else {
        gSpuIrqAddr = 0;
    }
    SpuSetIRQAddr(gSpuIrqAddr);
    SpuSetIRQ(SPU_ON);
}

int StopMovie(void)
{
    DecDCToutCallback(0);
    StUnSetRing();
    if (gMovieState == MOVIE_PLAYING) {
        if (CdSync(1, 0) == 0) {
            CdControlF(CdlPause, 0);
        }
        gMovieState = MOVIE_DECODE_STILL;
    }
    FreeHeapBlock(gStreamRing);
    FreeHeapBlock(sMovieDecoder.runLevel[0]);
    FreeHeapBlock(sMovieDecoder.runLevel[1]);
    FreeHeapBlock(sMovieDecoder.slice[0]);
    return FreeHeapBlock(sMovieDecoder.slice[1]);
}

void PlayIdleMedia(void)
{
    SpuCommonAttr attr;

    gSiteParts[0] = gSiteAParts;
    gSiteParts[1] = gSiteBParts;
    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
                SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    attr.mvol.left = 0x4fff;
    attr.mvol.right = 0x4fff;
    attr.mvolmode.left = 0x4fff;
    attr.mvolmode.right = 0x4fff;
    attr.cd.volume.left = 0;
    attr.cd.volume.right = 0;
    attr.cd.mix = SPU_ON;
    SpuSetCommonAttr(&attr);
    if ((rand() & 1) == 0) {
        PlayIdleMovie();
    } else {
        InitTileGrid("", 351);
        PlayIdleAudio();
        FreeTileGrid();
    }
}

int RestartXaRead(int command)
{
    CdlLOC start;

    CdIntToPos(gXaStartSector, &start);
    CdControl(CdlReadS, 0, 0);
    gXaReading = 1;
    gXaSector = gXaStartSector;
    return 0;
}

int GetFilePartSize(FilePart *parts, int part)
{
    return parts[part].size;
}

void LoadMediaPlayerHudTim(int unused, int x, int y, int clutY)
{
    RECT rect;
    void *buffer;
    int request;
    u_long *image;
    GsIMAGE *tim;

    buffer = AllocHeapBlock(gBinParts[21].size);
    request = RequestFilePartRead(3, 21, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
    }
    image = (u_long *)UnpackNapk(buffer, gBinParts[21].size);
    FreeHeapBlock(buffer);
    image++;
    tim = &sTimInfo;
    GsGetTimInfo(image, tim);
    setRECT(&rect, x, y, tim->pw, tim->ph);
    LoadImage(&rect, tim->pixel);
    DrawSync(0);
    if ((sTimInfo.pmode >> 3) & 1) {
        setRECT(&rect, 448, clutY + 71, tim->cw, tim->ch);
        LoadImage(&rect, tim->clut);
        DrawSync(0);
    }
    FreeHeapBlock(image - 1);
}

int gXaStartSector;
int gXaSector;
int gXaEndSector;

int ReadSitePart(int part, void *dest)
{
    unsigned short siteFile[2] = {6, 6};
    int disc = gCurrentDisc;
    int task;
    int status;

    task = RequestFilePartRead(siteFile[disc], part, gSiteParts[disc], dest);
    while ((status = PollCdRequest(task)) == 0) {
        DrawLoadingFrame(0);
    }
    return status;
}

int ReadSiteBPart(int part, void *dest)
{
    unsigned short siteFile[2] = {6, 6};
    int task;
    int status;

    task = RequestFilePartRead(siteFile[0], part, gSiteParts[1], dest);
    do {
        status = PollCdRequest(task);
    } while (status == 0);
    return status;
}

int StartXaRead(int start, int end)
{
    u_char mode;

    gXaStartSector = start;
    gXaSector = start;
    gXaEndSector = end;
    mode = CdlModeSpeed | CdlModeRT | CdlModeSF;
    CdControlB(CdlSetmode, &mode, 0);
    RestartXaRead(CdlReadS);
    if (gTrackIndex < 518) {
        SpuSetIRQ(SPU_ON);
    }
    return 0;
}

void MuteCdAudio(void)
{
    SpuCommonAttr attr;

    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
                SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    attr.mvol.left = 0x4fff;
    attr.mvol.right = 0x4fff;
    attr.mvolmode.left = 0x4fff;
    attr.mvolmode.right = 0x4fff;
    attr.cd.volume.left = 0;
    attr.cd.volume.right = 0;
    attr.cd.mix = SPU_ON;
    SpuSetCommonAttr(&attr);
}

void UnmuteCdAudio(void)
{
    SpuCommonAttr attr;

    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
                SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    attr.mvol.left = 0x3fff;
    attr.mvol.right = 0x3fff;
    attr.mvolmode.left = 0x3fff;
    attr.mvolmode.right = 0x3fff;
    attr.cd.volume.left = 0x3fff;
    attr.cd.volume.right = 0x3fff;
    attr.cd.mix = SPU_ON;
    SpuSetCommonAttr(&attr);
}

int GetCdPeakLevel(void)
{
    short peak = 0;
    int start, end, i;
    short sample;

    if (gSpuIrqAddr == 0) {
        start = 0;
        end = 256;
    } else {
        start = 256;
        end = 512;
    }
    for (i = start; i < end - 50; i++) {
        sample = ((short *)gDecodedAudio)[i];
        if (sample < 0) {
            sample = -sample;
        }
        if (peak < sample) {
            peak = sample;
        }
    }
    return (short)(peak / 4681);
}

short GetCdPeakLevel5(void)
{
    int start, end, i;
    short peak = 0;
    short sample;

    if (gSpuIrqAddr == 0) {
        start = 0;
        end = 256;
    } else {
        start = 256;
        end = 512;
    }
    for (i = start; i < end - 50; i++) {
        sample = ((short *)gDecodedAudio)[i];
        if (sample < 0) {
            sample = -sample;
        }
        if (peak < sample) {
            peak = sample;
        }
    }
    return peak / 0x1999;
}

int SeekXaFile(void)
{
    CdlFILE file;

    CdIntToPos(gDiscFiles[gXaFiles[gTrackXaFile - 1]].sector, &file.pos);
    CdControl(CdlSetloc, (u_char *)&file.pos, 0);
    return CdControl(CdlSeekP, (u_char *)&file.pos, 0);
}

int InitSprite(GsSPRITE *sprite, int x, int y, int w, int h,
                  int tpage, int u, int v, int cx, int cy,
                  u_long attribute, int mx, int my)
{
    sprite->x = x;
    sprite->y = y;
    sprite->w = w;
    sprite->r = 128;
    sprite->g = 128;
    sprite->b = 128;
    sprite->h = h;
    sprite->tpage = tpage;
    sprite->u = u;
    sprite->v = v;
    sprite->cx = cx;
    sprite->cy = cy;
    sprite->mx = mx;
    sprite->my = my;
    sprite->attribute = attribute;
}

void InitSpriteQuad(SpriteQuad *quad, int x, int y, int w, int h,
                   int tpage, int u, int v, int clut,
                   u_long attribute)
{
    quad->x = x;
    quad->y = y;
    quad->w = w;
    quad->r = 128;
    quad->g = 128;
    quad->b = 128;
    quad->h = h;
    quad->tpage = tpage;
    quad->u = u;
    quad->v = v;
    quad->clut = clut;
    quad->attribute = attribute;
}

int IsTrackDone(void)
{
    return gTrackDone;
}

DiscFile gDiscFiles[100] = {
    { 23, 618496 },
    { 325, 72 },
    { 326, 20 },
    { 327, 256000 },
    { 452, 13936640 },
    { 7257, 438272 },
    { 7471, 7057408 },
    { 10917, 14690304 },
    { 18092, 1569792 },
    { 18764, 1569792 },
    { 19436, 1719296 },
    { 20172, 1569792 },
    { 20844, 2616320 },
    { 21964, 1345536 },
    { 22540, 1569792 },
    { 23212, 1196032 },
    { 23724, 1196032 },
    { 24236, 672768 },
    { 24524, 1196032 },
    { 25036, 822272 },
    { 25388, 1569792 },
    { 26060, 1868800 },
    { 26860, 1868800 },
    { 27660, 1868800 },
    { 28460, 1719296 },
    { 29196, 1569792 },
    { 29868, 1868800 },
    { 30668, 1719296 },
    { 31404, 1569792 },
    { 32076, 2242560 },
    { 33036, 1420288 },
    { 33644, 5980160 },
    { 36204, 2317312 },
    { 37196, 2317312 },
    { 38188, 4335616 },
    { 40044, 3363840 },
    { 41484, 3363840 },
    { 42924, 8820736 },
    { 46700, 4260864 },
    { 48524, 3214336 },
    { 49900, 3924480 },
    { 51580, 11810816 },
    { 56636, 9867264 },
    { 60860, 10764288 },
    { 65468, 6204416 },
    { 68124, 2691072 },
    { 69276, 5830656 },
    { 71772, 13530112 },
    { 77564, 4228160 },
    { 79374, 7400448 },
    { 82542, 9119744 },
    { 86446, 5307392 },
    { 88718, 5755904 },
    { 91182, 5606400 },
    { 93582, 4260864 },
    { 95406, 14726144 },
    { 101711, 71163904 },
    { 132175, 17043456 },
    { 139471, 17491968 },
    { 146959, 22799360 },
    { 156719, 19659776 },
    { 165135, 23920640 },
    { 175375, 40291328 },
    { 192623, 29153280 },
    { 205103, 2840576 },
    { 206319, 2093056 },
    { 207215, 3214336 },
    { 208591, 16295936 },
    { 215567, 75350016 },
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
};
DiscFile gDisc1Files[69] = {
    { 23, 618496 },
    { 325, 72 },
    { 326, 20 },
    { 327, 256000 },
    { 452, 13936640 },
    { 7257, 438272 },
    { 7471, 7057408 },
    { 10917, 14690304 },
    { 18092, 1569792 },
    { 18764, 1569792 },
    { 19436, 1719296 },
    { 20172, 1569792 },
    { 20844, 2616320 },
    { 21964, 1345536 },
    { 22540, 1569792 },
    { 23212, 1196032 },
    { 23724, 1196032 },
    { 24236, 672768 },
    { 24524, 1196032 },
    { 25036, 822272 },
    { 25388, 1569792 },
    { 26060, 1868800 },
    { 26860, 1868800 },
    { 27660, 1868800 },
    { 28460, 1719296 },
    { 29196, 1569792 },
    { 29868, 1868800 },
    { 30668, 1719296 },
    { 31404, 1569792 },
    { 32076, 2242560 },
    { 33036, 1420288 },
    { 33644, 5980160 },
    { 36204, 2317312 },
    { 37196, 2317312 },
    { 38188, 4335616 },
    { 40044, 3363840 },
    { 41484, 3363840 },
    { 42924, 8820736 },
    { 46700, 4260864 },
    { 48524, 3214336 },
    { 49900, 3924480 },
    { 51580, 11810816 },
    { 56636, 9867264 },
    { 60860, 10764288 },
    { 65468, 6204416 },
    { 68124, 2691072 },
    { 69276, 5830656 },
    { 71772, 13530112 },
    { 77564, 4228160 },
    { 79374, 7400448 },
    { 82542, 9119744 },
    { 86446, 5307392 },
    { 88718, 5755904 },
    { 91182, 5606400 },
    { 93582, 4260864 },
    { 95406, 14726144 },
    { 101711, 71163904 },
    { 132175, 17043456 },
    { 139471, 17491968 },
    { 146959, 22799360 },
    { 156719, 19659776 },
    { 165135, 23920640 },
    { 175375, 40291328 },
    { 192623, 29153280 },
    { 205103, 2840576 },
    { 206319, 2093056 },
    { 207215, 3214336 },
    { 208591, 16295936 },
    { 215567, 75350016 },
};
DiscFile gDisc2Files[57] = {
    { 23, 618496 },
    { 325, 72 },
    { 326, 20 },
    { 327, 256000 },
    { 452, 13936640 },
    { 7257, 438272 },
    { 7471, 5328896 },
    { 10073, 14690304 },
    { 17248, 1569792 },
    { 17920, 1569792 },
    { 18592, 1719296 },
    { 19328, 1569792 },
    { 20000, 2616320 },
    { 21120, 1345536 },
    { 21696, 1569792 },
    { 22368, 512000 },
    { 22618, 1536000 },
    { 23368, 5232640 },
    { 25608, 3887104 },
    { 27272, 9568256 },
    { 31368, 5157888 },
    { 33576, 8820736 },
    { 37352, 7774208 },
    { 40680, 3513344 },
    { 42184, 5307392 },
    { 44456, 15473664 },
    { 51080, 12633088 },
    { 56488, 6204416 },
    { 59144, 12184576 },
    { 64360, 33101120 },
    { 78530, 7176192 },
    { 81602, 8970240 },
    { 85442, 5456896 },
    { 87778, 5830656 },
    { 90274, 2018304 },
    { 91138, 4260864 },
    { 92962, 5980160 },
    { 95522, 17043456 },
    { 102818, 21080064 },
    { 111842, 4260864 },
    { 113666, 12053760 },
    { 118826, 14875648 },
    { 125194, 10913792 },
    { 129866, 12633088 },
    { 135274, 13380608 },
    { 141002, 9867264 },
    { 145226, 12707840 },
    { 150666, 11287552 },
    { 155499, 61595648 },
    { 181867, 76471296 },
    { 214603, 23995392 },
    { 224875, 15697920 },
    { 231595, 33937408 },
    { 246123, 35133440 },
    { 261163, 2840576 },
    { 262379, 16295936 },
    { 269356, 111529984 },
};
Track gTracks[736] = {
    { 0, 0, 30, 60 },
    { 0, 0, 31, 255 },
    { 0, 0, 32, 98 },
    { 0, 0, 33, 96 },
    { 0, 0, 34, 183 },
    { 0, 0, 35, 141 },
    { 0, 0, 36, 142 },
    { 0, 0, 37, 375 },
    { 0, 0, 38, 180 },
    { 0, 0, 39, 135 },
    { 0, 0, 40, 168 },
    { 0, 0, 41, 505 },
    { 0, 0, 42, 420 },
    { 0, 0, 43, 458 },
    { 0, 0, 44, 263 },
    { 0, 0, 45, 113 },
    { 0, 0, 46, 248 },
    { 0, 0, 47, 576 },
    { 0, 0, 48, 181 },
    { 0, 0, 49, 315 },
    { 0, 0, 50, 390 },
    { 0, 0, 51, 227 },
    { 0, 0, 52, 246 },
    { 0, 0, 53, 237 },
    { 0, 0, 17, 224 },
    { 0, 0, 18, 165 },
    { 0, 0, 54, 180 },
    { 0, 0, 55, 624 },
    { 0, 0, 19, 408 },
    { 0, 0, 20, 220 },
    { 0, 0, 21, 377 },
    { 0, 0, 22, 331 },
    { 0, 0, 23, 149 },
    { 0, 0, 24, 225 },
    { 0, 0, 25, 660 },
    { 0, 0, 26, 540 },
    { 0, 0, 27, 262 },
    { 0, 0, 28, 521 },
    { 0, 0, 29, 1417 },
    { 0, 0, 30, 305 },
    { 0, 0, 31, 383 },
    { 0, 0, 32, 233 },
    { 0, 0, 33, 246 },
    { 0, 0, 34, 83 },
    { 0, 0, 35, 181 },
    { 0, 0, 36, 255 },
    { 0, 0, 37, 728 },
    { 0, 0, 38, 886 },
    { 0, 0, 39, 166 },
    { 0, 0, 40, 516 },
    { 0, 0, 41, 635 },
    { 0, 0, 42, 464 },
    { 0, 0, 43, 540 },
    { 0, 0, 44, 570 },
    { 0, 0, 45, 420 },
    { 0, 0, 46, 542 },
    { 0, 0, 47, 480 },
    { 1, 0, 0, 943744 },
    { 1, 0, 1, 1086240 },
    { 1, 0, 2, 1761344 },
    { 1, 0, 3, 1457664 },
    { 1, 0, 4, 1375904 },
    { 1, 0, 5, 1065216 },
    { 1, 0, 6, 1359552 },
    { 1, 0, 7, 1074560 },
    { 1, 0, 8, 553632 },
    { 1, 0, 9, 1850112 },
    { 1, 0, 10, 1903840 },
    { 1, 0, 11, 1777696 },
    { 1, 0, 12, 1878144 },
    { 1, 0, 13, 2354688 },
    { 1, 0, 14, 1770688 },
    { 1, 0, 15, 1284800 },
    { 1, 0, 16, 1298816 },
    { 1, 0, 17, 1198368 },
    { 1, 0, 18, 2044000 },
    { 1, 0, 19, 1677248 },
    { 1, 0, 20, 1539424 },
    { 1, 0, 21, 1698272 },
    { 1, 0, 22, 899360 },
    { 1, 0, 23, 822272 },
    { 1, 0, 24, 1422624 },
    { 1, 0, 25, 1653888 },
    { 1, 0, 26, 978784 },
    { 1, 0, 27, 992800 },
    { 1, 0, 28, 854976 },
    { 1, 0, 29, 810592 },
    { 1, 0, 30, 2233216 },
    { 1, 0, 31, 1249760 },
    { 2, 0, 0, 2223872 },
    { 2, 0, 1, 1460000 },
    { 2, 0, 2, 2123424 },
    { 2, 0, 3, 1338528 },
    { 2, 0, 4, 1607168 },
    { 2, 0, 5, 1457664 },
    { 14, 0, 0, 1924864 },
    { 14, 0, 1, 941408 },
    { 14, 0, 2, 831616 },
    { 14, 0, 3, 815264 },
    { 14, 0, 4, 1039520 },
    { 14, 0, 5, 700800 },
    { 14, 0, 6, 359744 },
    { 14, 0, 7, 1588480 },
    { 14, 0, 8, 1139968 },
    { 14, 0, 9, 1025504 },
    { 14, 0, 10, 703136 },
    { 14, 0, 11, 943744 },
    { 14, 0, 12, 1179680 },
    { 14, 0, 13, 640064 },
    { 14, 0, 14, 892352 },
    { 2, 0, 6, 401792 },
    { 2, 0, 7, 427488 },
    { 2, 0, 8, 892352 },
    { 2, 0, 9, 439168 },
    { 2, 0, 10, 378432 },
    { 2, 0, 11, 565312 },
    { 2, 0, 12, 1146976 },
    { 2, 0, 13, 532608 },
    { 2, 0, 14, 338720 },
    { 2, 0, 15, 266304 },
    { 2, 0, 16, 126144 },
    { 2, 0, 17, 415808 },
    { 2, 0, 18, 464864 },
    { 2, 0, 19, 387776 },
    { 2, 0, 20, 726496 },
    { 2, 0, 21, 327040 },
    { 2, 0, 22, 126144 },
    { 2, 0, 23, 735840 },
    { 2, 0, 24, 249952 },
    { 2, 0, 25, 329376 },
    { 2, 0, 26, 371424 },
    { 2, 0, 27, 301344 },
    { 2, 0, 28, 350400 },
    { 2, 0, 29, 226592 },
    { 2, 0, 30, 380768 },
    { 2, 0, 31, 420480 },
    { 3, 0, 0, 233600 },
    { 3, 0, 1, 161184 },
    { 3, 0, 2, 357408 },
    { 14, 0, 15, 112128 },
    { 14, 0, 16, 198560 },
    { 14, 0, 17, 172864 },
    { 14, 0, 18, 462528 },
    { 14, 0, 19, 343392 },
    { 14, 0, 20, 427488 },
    { 14, 0, 21, 422816 },
    { 14, 0, 22, 457856 },
    { 14, 0, 23, 376096 },
    { 14, 0, 24, 317696 },
    { 14, 0, 25, 350400 },
    { 14, 0, 26, 313024 },
    { 14, 0, 27, 485888 },
    { 14, 0, 28, 467200 },
    { 14, 0, 29, 175200 },
    { 14, 0, 30, 296672 },
    { 14, 0, 31, 137824 },
    { 15, 0, 0, 217248 },
    { 15, 0, 1, 224256 },
    { 15, 0, 2, 595680 },
    { 15, 0, 3, 595680 },
    { 15, 0, 4, 925056 },
    { 15, 0, 5, 932064 },
    { 15, 0, 6, 2389728 },
    { 3, 0, 3, 308352 },
    { 3, 0, 4, 182208 },
    { 3, 0, 5, 317696 },
    { 3, 0, 6, 329376 },
    { 3, 0, 7, 306016 },
    { 3, 0, 8, 460192 },
    { 13, 0, 19, 86432 },
    { 13, 0, 20, 79424 },
    { 13, 0, 21, 112128 },
    { 13, 0, 22, 228928 },
    { 13, 0, 23, 212576 },
    { 13, 0, 24, 420480 },
    { 13, 0, 25, 245280 },
    { 13, 0, 26, 418144 },
    { 13, 0, 27, 205568 },
    { 13, 0, 28, 509248 },
    { 15, 0, 7, 1020832 },
    { 15, 0, 8, 679776 },
    { 15, 0, 9, 219584 },
    { 15, 0, 10, 434496 },
    { 15, 0, 11, 303680 },
    { 15, 0, 12, 327040 },
    { 15, 0, 13, 707808 },
    { 15, 0, 14, 427488 },
    { 15, 0, 15, 516256 },
    { 15, 0, 16, 544288 },
    { 3, 0, 9, 471872 },
    { 3, 0, 10, 324704 },
    { 3, 0, 11, 355072 },
    { 3, 0, 12, 182208 },
    { 3, 0, 13, 401792 },
    { 3, 0, 14, 35040 },
    { 3, 0, 15, 299008 },
    { 3, 0, 16, 170528 },
    { 3, 0, 17, 238272 },
    { 3, 0, 18, 161184 },
    { 3, 0, 19, 156512 },
    { 3, 0, 20, 170528 },
    { 3, 0, 21, 226592 },
    { 3, 0, 22, 263968 },
    { 3, 0, 23, 179872 },
    { 3, 0, 24, 245280 },
    { 3, 0, 25, 259296 },
    { 3, 0, 26, 179872 },
    { 3, 0, 27, 532608 },
    { 3, 0, 28, 380768 },
    { 3, 0, 29, 126144 },
    { 3, 0, 30, 212576 },
    { 3, 0, 31, 189216 },
    { 4, 0, 0, 303680 },
    { 4, 0, 1, 93440 },
    { 4, 0, 2, 266304 },
    { 4, 0, 3, 397120 },
    { 4, 0, 4, 322368 },
    { 4, 0, 5, 30368 },
    { 4, 0, 6, 21024 },
    { 4, 0, 7, 336384 },
    { 4, 0, 8, 205568 },
    { 4, 0, 9, 289664 },
    { 4, 0, 10, 219584 },
    { 4, 0, 11, 165856 },
    { 4, 0, 12, 203232 },
    { 4, 0, 13, 88768 },
    { 4, 0, 14, 231264 },
    { 4, 0, 15, 369088 },
    { 4, 0, 16, 256960 },
    { 4, 0, 17, 338720 },
    { 4, 0, 18, 394784 },
    { 4, 0, 19, 86432 },
    { 4, 0, 20, 527936 },
    { 4, 0, 21, 247616 },
    { 4, 0, 22, 88768 },
    { 4, 0, 23, 504576 },
    { 4, 0, 24, 380768 },
    { 4, 0, 25, 315360 },
    { 4, 0, 26, 161184 },
    { 4, 0, 27, 345728 },
    { 4, 0, 28, 128480 },
    { 4, 0, 29, 546624 },
    { 4, 0, 30, 331712 },
    { 4, 0, 31, 126144 },
    { 5, 0, 0, 214912 },
    { 5, 0, 1, 422816 },
    { 5, 0, 2, 712480 },
    { 5, 0, 3, 130816 },
    { 5, 0, 4, 299008 },
    { 5, 0, 5, 434496 },
    { 5, 0, 6, 240608 },
    { 5, 0, 7, 49056 },
    { 5, 0, 8, 371424 },
    { 5, 0, 9, 126144 },
    { 5, 0, 10, 362080 },
    { 5, 0, 11, 228928 },
    { 5, 0, 12, 275648 },
    { 5, 0, 13, 352736 },
    { 5, 0, 14, 310688 },
    { 5, 0, 15, 527936 },
    { 5, 0, 16, 60736 },
    { 5, 0, 17, 296672 },
    { 5, 0, 18, 37376 },
    { 5, 0, 19, 254624 },
    { 5, 0, 20, 317696 },
    { 5, 0, 21, 86432 },
    { 5, 0, 22, 464864 },
    { 5, 0, 23, 84096 },
    { 5, 0, 24, 182208 },
    { 5, 0, 25, 70080 },
    { 5, 0, 26, 301344 },
    { 5, 0, 27, 161184 },
    { 5, 0, 28, 287328 },
    { 5, 0, 29, 72416 },
    { 5, 0, 30, 476544 },
    { 5, 0, 31, 142496 },
    { 6, 0, 0, 327040 },
    { 6, 0, 1, 79424 },
    { 6, 0, 2, 88768 },
    { 6, 0, 3, 378432 },
    { 6, 0, 4, 310688 },
    { 6, 0, 5, 109792 },
    { 6, 0, 6, 481216 },
    { 6, 0, 7, 126144 },
    { 6, 0, 8, 294336 },
    { 6, 0, 9, 105120 },
    { 6, 0, 10, 299008 },
    { 6, 0, 11, 37376 },
    { 6, 0, 12, 366752 },
    { 6, 0, 13, 425152 },
    { 6, 0, 14, 614368 },
    { 6, 0, 15, 112128 },
    { 6, 0, 16, 443840 },
    { 6, 0, 17, 142496 },
    { 6, 0, 18, 497568 },
    { 6, 0, 19, 277984 },
    { 6, 0, 20, 476544 },
    { 6, 0, 21, 460192 },
    { 6, 0, 22, 114464 },
    { 6, 0, 23, 284992 },
    { 6, 0, 24, 72416 },
    { 6, 0, 25, 182208 },
    { 6, 0, 26, 270976 },
    { 6, 0, 27, 385440 },
    { 6, 0, 28, 128480 },
    { 6, 0, 29, 366752 },
    { 6, 0, 30, 49056 },
    { 6, 0, 31, 385440 },
    { 7, 0, 0, 121472 },
    { 7, 0, 1, 245280 },
    { 7, 0, 2, 154176 },
    { 7, 0, 3, 338720 },
    { 7, 0, 4, 128480 },
    { 7, 0, 5, 373760 },
    { 7, 0, 6, 60736 },
    { 7, 0, 7, 191552 },
    { 7, 0, 8, 130816 },
    { 7, 0, 9, 219584 },
    { 7, 0, 10, 156512 },
    { 7, 0, 11, 273312 },
    { 7, 0, 12, 91104 },
    { 7, 0, 13, 747520 },
    { 7, 0, 14, 170528 },
    { 7, 0, 15, 214912 },
    { 7, 0, 16, 121472 },
    { 7, 0, 17, 404128 },
    { 7, 0, 18, 394784 },
    { 7, 0, 19, 77088 },
    { 7, 0, 20, 233600 },
    { 7, 0, 21, 112128 },
    { 7, 0, 22, 532608 },
    { 7, 0, 23, 114464 },
    { 7, 0, 24, 607360 },
    { 7, 0, 25, 275648 },
    { 7, 0, 26, 189216 },
    { 7, 0, 27, 324704 },
    { 7, 0, 28, 35040 },
    { 7, 0, 29, 439168 },
    { 7, 0, 30, 154176 },
    { 15, 0, 17, 341056 },
    { 15, 0, 18, 70080 },
    { 15, 0, 19, 378432 },
    { 15, 0, 20, 163520 },
    { 15, 0, 21, 637728 },
    { 15, 0, 22, 79424 },
    { 15, 0, 23, 280320 },
    { 15, 0, 24, 126144 },
    { 15, 0, 25, 357408 },
    { 15, 0, 26, 107456 },
    { 15, 0, 27, 315360 },
    { 15, 0, 28, 53728 },
    { 15, 0, 29, 406464 },
    { 15, 0, 30, 18688 },
    { 15, 0, 31, 294336 },
    { 16, 0, 0, 51392 },
    { 16, 0, 1, 249952 },
    { 16, 0, 2, 126144 },
    { 16, 0, 3, 345728 },
    { 16, 0, 4, 91104 },
    { 16, 0, 5, 263968 },
    { 16, 0, 6, 196224 },
    { 16, 0, 7, 296672 },
    { 16, 0, 8, 72416 },
    { 16, 0, 9, 359744 },
    { 16, 0, 10, 72416 },
    { 16, 0, 11, 749856 },
    { 16, 0, 12, 418144 },
    { 16, 0, 13, 140160 },
    { 16, 0, 14, 126144 },
    { 16, 0, 15, 177536 },
    { 16, 0, 16, 350400 },
    { 16, 0, 17, 93440 },
    { 16, 0, 18, 308352 },
    { 16, 0, 19, 170528 },
    { 16, 0, 20, 385440 },
    { 16, 0, 21, 275648 },
    { 16, 0, 22, 334048 },
    { 16, 0, 23, 280320 },
    { 16, 0, 24, 93440 },
    { 16, 0, 25, 203232 },
    { 16, 0, 26, 226592 },
    { 16, 0, 27, 221920 },
    { 16, 0, 28, 198560 },
    { 16, 0, 29, 303680 },
    { 16, 0, 30, 81760 },
    { 16, 0, 31, 280320 },
    { 17, 0, 0, 334048 },
    { 17, 0, 1, 212576 },
    { 17, 0, 2, 261632 },
    { 17, 0, 3, 350400 },
    { 17, 0, 4, 165856 },
    { 17, 0, 5, 205568 },
    { 17, 0, 6, 98112 },
    { 17, 0, 7, 469536 },
    { 17, 0, 8, 142496 },
    { 17, 0, 9, 72416 },
    { 17, 0, 10, 77088 },
    { 17, 0, 11, 266304 },
    { 17, 0, 12, 249952 },
    { 17, 0, 13, 107456 },
    { 17, 0, 14, 378432 },
    { 17, 0, 15, 133152 },
    { 17, 0, 16, 79424 },
    { 17, 0, 17, 308352 },
    { 17, 0, 18, 112128 },
    { 17, 0, 19, 490560 },
    { 17, 0, 20, 231264 },
    { 17, 0, 21, 98112 },
    { 17, 0, 22, 242944 },
    { 17, 0, 23, 371424 },
    { 17, 0, 24, 114464 },
    { 17, 0, 25, 221920 },
    { 17, 0, 26, 95776 },
    { 17, 0, 27, 380768 },
    { 17, 0, 28, 156512 },
    { 17, 0, 29, 116800 },
    { 17, 0, 30, 121472 },
    { 17, 0, 31, 46720 },
    { 18, 0, 0, 114464 },
    { 18, 0, 1, 238272 },
    { 18, 0, 2, 175200 },
    { 18, 0, 3, 191552 },
    { 18, 0, 4, 296672 },
    { 18, 0, 5, 221920 },
    { 18, 0, 6, 182208 },
    { 18, 0, 7, 843296 },
    { 7, 0, 31, 462528 },
    { 8, 0, 0, 133152 },
    { 8, 0, 1, 523264 },
    { 8, 0, 2, 616704 },
    { 8, 0, 3, 679776 },
    { 8, 0, 4, 541952 },
    { 8, 0, 5, 235936 },
    { 8, 0, 6, 355072 },
    { 8, 0, 7, 383104 },
    { 8, 0, 8, 287328 },
    { 8, 0, 9, 904032 },
    { 8, 0, 10, 876000 },
    { 8, 0, 11, 420480 },
    { 8, 0, 12, 616704 },
    { 8, 0, 13, 1259104 },
    { 8, 0, 14, 254624 },
    { 8, 0, 15, 675104 },
    { 8, 0, 16, 658752 },
    { 8, 0, 17, 308352 },
    { 8, 0, 18, 266304 },
    { 8, 0, 19, 518592 },
    { 8, 0, 20, 313024 },
    { 8, 0, 21, 369088 },
    { 8, 0, 22, 703136 },
    { 8, 0, 23, 411136 },
    { 8, 0, 24, 548960 },
    { 8, 0, 25, 273312 },
    { 8, 0, 26, 978784 },
    { 8, 0, 27, 955424 },
    { 8, 0, 28, 317696 },
    { 8, 0, 29, 887680 },
    { 8, 0, 30, 415808 },
    { 8, 0, 31, 471872 },
    { 9, 0, 0, 756864 },
    { 9, 0, 1, 292000 },
    { 9, 0, 2, 876000 },
    { 9, 0, 3, 710144 },
    { 9, 0, 4, 747520 },
    { 9, 0, 5, 609696 },
    { 9, 0, 6, 373760 },
    { 9, 0, 7, 39712 },
    { 9, 0, 8, 700800 },
    { 9, 0, 9, 911040 },
    { 9, 0, 10, 565312 },
    { 18, 0, 8, 394784 },
    { 18, 0, 9, 270976 },
    { 18, 0, 10, 939072 },
    { 18, 0, 11, 815264 },
    { 18, 0, 12, 920384 },
    { 18, 0, 13, 352736 },
    { 18, 0, 14, 44384 },
    { 18, 0, 15, 679776 },
    { 18, 0, 16, 366752 },
    { 18, 0, 17, 485888 },
    { 18, 0, 18, 119136 },
    { 18, 0, 19, 784896 },
    { 18, 0, 20, 387776 },
    { 18, 0, 21, 537280 },
    { 18, 0, 22, 144832 },
    { 18, 0, 23, 922720 },
    { 18, 0, 24, 572320 },
    { 18, 0, 25, 1060544 },
    { 18, 0, 26, 408800 },
    { 18, 0, 27, 915712 },
    { 18, 0, 28, 915712 },
    { 18, 0, 29, 843296 },
    { 18, 0, 30, 289664 },
    { 18, 0, 31, 623712 },
    { 19, 0, 0, 1097920 },
    { 19, 0, 1, 831616 },
    { 19, 0, 2, 135488 },
    { 19, 0, 3, 453184 },
    { 19, 0, 4, 448512 },
    { 19, 0, 5, 277984 },
    { 19, 0, 6, 343392 },
    { 19, 0, 7, 380768 },
    { 19, 0, 8, 415808 },
    { 19, 0, 9, 212576 },
    { 19, 0, 10, 815264 },
    { 19, 0, 11, 142496 },
    { 19, 0, 12, 343392 },
    { 19, 0, 13, 450848 },
    { 19, 0, 14, 179872 },
    { 19, 0, 15, 432160 },
    { 19, 0, 16, 287328 },
    { 19, 0, 17, 348064 },
    { 19, 0, 18, 238272 },
    { 19, 0, 19, 182208 },
    { 19, 0, 20, 259296 },
    { 19, 0, 21, 345728 },
    { 19, 0, 22, 338720 },
    { 19, 0, 23, 1065216 },
    { 9, 0, 11, 14016 },
    { 9, 0, 12, 14016 },
    { 9, 0, 13, 18688 },
    { 9, 0, 14, 16352 },
    { 9, 0, 15, 21024 },
    { 9, 0, 16, 14016 },
    { 9, 0, 17, 7008 },
    { 9, 0, 18, 9344 },
    { 9, 0, 19, 11680 },
    { 9, 0, 20, 32704 },
    { 9, 0, 21, 16352 },
    { 9, 0, 22, 35040 },
    { 9, 0, 23, 42048 },
    { 9, 0, 24, 35040 },
    { 9, 0, 25, 11680 },
    { 9, 0, 26, 46720 },
    { 9, 0, 27, 37376 },
    { 9, 0, 28, 16352 },
    { 9, 0, 29, 18688 },
    { 9, 0, 30, 39712 },
    { 9, 0, 31, 30368 },
    { 10, 0, 0, 65408 },
    { 10, 0, 1, 30368 },
    { 10, 0, 2, 18688 },
    { 10, 0, 3, 37376 },
    { 10, 0, 4, 88768 },
    { 10, 0, 5, 14016 },
    { 10, 0, 6, 11680 },
    { 10, 0, 7, 56064 },
    { 10, 0, 8, 37376 },
    { 10, 0, 9, 21024 },
    { 10, 0, 10, 32704 },
    { 10, 0, 11, 28032 },
    { 10, 0, 12, 56064 },
    { 10, 0, 13, 9344 },
    { 10, 0, 14, 16352 },
    { 10, 0, 15, 37376 },
    { 10, 0, 16, 9344 },
    { 10, 0, 17, 25696 },
    { 10, 0, 18, 11680 },
    { 10, 0, 19, 32704 },
    { 10, 0, 20, 30368 },
    { 10, 0, 21, 14016 },
    { 10, 0, 22, 11680 },
    { 10, 0, 23, 16352 },
    { 10, 0, 24, 14016 },
    { 10, 0, 25, 25696 },
    { 10, 0, 26, 74752 },
    { 10, 0, 27, 11680 },
    { 10, 0, 28, 21024 },
    { 10, 0, 29, 39712 },
    { 10, 0, 30, 46720 },
    { 10, 0, 31, 16352 },
    { 11, 0, 0, 18688 },
    { 11, 0, 1, 18688 },
    { 11, 0, 2, 28032 },
    { 11, 0, 3, 32704 },
    { 11, 0, 4, 28032 },
    { 11, 0, 5, 51392 },
    { 11, 0, 6, 28032 },
    { 11, 0, 7, 53728 },
    { 11, 0, 8, 18688 },
    { 11, 0, 9, 25696 },
    { 11, 0, 10, 32704 },
    { 11, 0, 11, 23360 },
    { 11, 0, 12, 9344 },
    { 11, 0, 13, 14016 },
    { 11, 0, 14, 11680 },
    { 11, 0, 15, 16352 },
    { 11, 0, 16, 9344 },
    { 11, 0, 17, 44384 },
    { 11, 0, 18, 21024 },
    { 11, 0, 19, 16352 },
    { 11, 0, 20, 21024 },
    { 11, 0, 21, 16352 },
    { 11, 0, 22, 42048 },
    { 11, 0, 23, 60736 },
    { 11, 0, 24, 46720 },
    { 11, 0, 25, 65408 },
    { 11, 0, 26, 21024 },
    { 11, 0, 27, 9344 },
    { 11, 0, 28, 28032 },
    { 11, 0, 29, 16352 },
    { 11, 0, 30, 28032 },
    { 11, 0, 31, 44384 },
    { 12, 0, 0, 58400 },
    { 12, 0, 1, 51392 },
    { 12, 0, 2, 37376 },
    { 12, 0, 3, 32704 },
    { 12, 0, 4, 18688 },
    { 19, 0, 24, 25696 },
    { 19, 0, 25, 18688 },
    { 19, 0, 26, 30368 },
    { 19, 0, 27, 14016 },
    { 19, 0, 28, 42048 },
    { 19, 0, 29, 14016 },
    { 19, 0, 30, 21024 },
    { 19, 0, 31, 37376 },
    { 20, 0, 0, 53728 },
    { 20, 0, 1, 49056 },
    { 20, 0, 2, 58400 },
    { 20, 0, 3, 16352 },
    { 20, 0, 4, 28032 },
    { 20, 0, 5, 53728 },
    { 20, 0, 6, 44384 },
    { 20, 0, 7, 44384 },
    { 20, 0, 8, 9344 },
    { 20, 0, 9, 44384 },
    { 20, 0, 10, 25696 },
    { 20, 0, 11, 18688 },
    { 20, 0, 12, 28032 },
    { 20, 0, 13, 37376 },
    { 20, 0, 14, 35040 },
    { 20, 0, 15, 75000 },
    { 20, 0, 16, 18688 },
    { 20, 0, 17, 28032 },
    { 20, 0, 18, 37376 },
    { 20, 0, 19, 11680 },
    { 20, 0, 20, 18688 },
    { 20, 0, 21, 49056 },
    { 20, 0, 22, 32704 },
    { 12, 0, 5, 9344 },
    { 12, 0, 6, 11680 },
    { 12, 0, 7, 16352 },
    { 12, 0, 8, 32704 },
    { 12, 0, 9, 42048 },
    { 12, 0, 10, 35040 },
    { 12, 0, 11, 42048 },
    { 12, 0, 12, 18688 },
    { 12, 0, 13, 39712 },
    { 12, 0, 14, 70080 },
    { 12, 0, 15, 39712 },
    { 12, 0, 16, 25696 },
    { 12, 0, 17, 37376 },
    { 12, 0, 18, 25696 },
    { 12, 0, 19, 16352 },
    { 12, 0, 20, 16352 },
    { 12, 0, 21, 100448 },
    { 12, 0, 22, 23360 },
    { 12, 0, 23, 46720 },
    { 12, 0, 24, 32704 },
    { 12, 0, 25, 32704 },
    { 12, 0, 26, 51392 },
    { 12, 0, 27, 25696 },
    { 12, 0, 28, 65408 },
    { 12, 0, 29, 18688 },
    { 12, 0, 30, 28032 },
    { 12, 0, 31, 23360 },
    { 13, 0, 0, 44384 },
    { 13, 0, 1, 21024 },
    { 13, 0, 2, 42048 },
    { 13, 0, 3, 70080 },
    { 13, 0, 4, 56064 },
    { 13, 0, 5, 67744 },
    { 13, 0, 6, 28032 },
    { 13, 0, 7, 23360 },
    { 13, 0, 8, 32704 },
    { 13, 0, 9, 70080 },
    { 13, 0, 10, 67744 },
    { 13, 0, 11, 39712 },
    { 13, 0, 12, 21024 },
    { 13, 0, 13, 32704 },
    { 13, 0, 14, 46720 },
    { 13, 0, 15, 21024 },
    { 13, 0, 16, 42048 },
    { 13, 0, 17, 67744 },
    { 13, 0, 18, 53728 },
    { 20, 0, 23, 86432 },
    { 20, 0, 24, 21024 },
    { 20, 0, 25, 42048 },
    { 20, 0, 26, 51392 },
    { 20, 0, 27, 53728 },
    { 20, 0, 28, 42048 },
    { 20, 0, 29, 11680 },
    { 20, 0, 30, 35040 },
    { 20, 0, 31, 42048 },
    { 21, 0, 0, 51392 },
    { 21, 0, 1, 63072 },
    { 21, 0, 2, 112128 },
    { 21, 0, 3, 23360 },
    { 21, 0, 4, 37376 },
    { 21, 0, 5, 86432 },
    { 21, 0, 6, 79424 },
    { 21, 0, 7, 112128 },
    { 21, 0, 8, 228928 },
    { 21, 0, 9, 212576 },
    { 21, 0, 10, 420480 },
    { 21, 0, 11, 245280 },
    { 21, 0, 12, 418144 },
    { 21, 0, 13, 205568 },
    { 21, 0, 14, 509248 },
    { 0, 0, 15, 50 },
    { 0, 0, 16, 49 },
    { 0, 0, 17, 26 },
    { 0, 0, 18, 50 },
    { 0, 0, 19, 35 },
    { 0, 0, 20, 65 },
    { 0, 0, 21, 80 },
    { 0, 0, 22, 80 },
    { 0, 0, 23, 80 },
    { 0, 0, 24, 70 },
    { 0, 0, 25, 65 },
    { 0, 0, 26, 80 },
    { 0, 0, 27, 73 },
    { 0, 0, 28, 65 },
    { 0, 0, 29, 95 },
    { 0, 0, 8, 65 },
    { 0, 0, 9, 65 },
    { 0, 0, 10, 73 },
    { 0, 0, 11, 65 },
    { 0, 0, 12, 110 },
    { 0, 0, 13, 56 },
    { 0, 0, 14, 63 },
    { 0, 0, 15, 25 },
    { 0, 0, 16, 75 },
    { 21, 0, 15, 18688 },
    { 21, 0, 16, 25696 },
    { 0, 0, 56, 4765 },
};
int gXaFiles[21] = {
    68, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 48, 49, 50, 51, 52, 53, 54, 55,
};
u_char gWordRing[30][2] = {
    { 40, 63 }, { 43, 63 }, { 50, 68 }, { 60, 76 }, { 71, 87 },
    { 89, 104 }, { 101, 117 }, { 112, 130 }, { 121, 140 }, { 129, 149 },
    { 133, 154 }, { 137, 158 }, { 139, 162 }, { 143, 165 }, { 145, 169 },
    { 147, 172 }, { 150, 177 }, { 152, 179 }, { 156, 184 }, { 157, 187 },
    { 160, 191 }, { 160, 197 }, { 144, 186 }, { 130, 173 }, { 114, 157 },
    { 97, 139 }, { 81, 121 }, { 64, 101 }, { 50, 82 }, { 42, 70 },
};
short gIdleTrackImageParts[2][10][3] = {
    {
        { 401, 404, 344 },
        { 320, 346, 706 },
        { 668, 667, 669 },
        { 664, 647, 655 },
        { 658, 630, 596 },
        { 504, 514, 539 },
        { 524, 476, 452 },
        { 400, 396, 405 },
        { 399, 398, 403 },
        { 402, 397, 416 },
    },
    {
        { 220, 232, 132 },
        { 122, 136, 464 },
        { 394, 389, 401 },
        { 387, 345, 350 },
        { 353, 333, 320 },
        { 294, 296, 298 },
        { 297, 289, 283 },
        { 218, 209, 234 },
        { 217, 214, 228 },
        { 224, 212, 242 },
    },
};
EndRollSegment gEndRollSegments[57] = {
    { 0, 180, 244 },
    { 0, 180, 408 },
    { 0, 409, 506 },
    { 0, 507, 555 },
    { 0, 556, 657 },
    { 0, 658, 674 },
    { 0, 675, 687 },
    { 0, 688, 696 },
    { 0, 697, 734 },
    { 0, 735, 766 },
    { 0, 767, 822 },
    { 0, 823, 872 },
    { 0, 873, 925 },
    { 0, 926, 1083 },
    { 0, 1084, 1125 },
    { 0, 1126, 1143 },
    { 0, 1144, 1185 },
    { 0, 1186, 1281 },
    { 0, 1282, 1374 },
    { 0, 1375, 1409 },
    { 0, 1410, 1480 },
    { 0, 1481, 1578 },
    { 0, 1579, 1701 },
    { 0, 1702, 1753 },
    { 0, 1754, 1823 },
    { 0, 1824, 1988 },
    { 0, 1989, 2078 },
    { 0, 2079, 2209 },
    { 0, 2210, 2385 },
    { 0, 2386, 2558 },
    { 0, 2559, 2690 },
    { 0, 2691, 2929 },
    { 0, 2930, 2964 },
    { 0, 2965, 3063 },
    { 0, 3064, 3139 },
    { 0, 3140, 3214 },
    { 0, 3215, 3258 },
    { 0, 3259, 3315 },
    { 0, 3316, 3446 },
    { 0, 3447, 3539 },
    { 0, 3540, 3632 },
    { 0, 3633, 3739 },
    { 0, 3740, 3853 },
    { 0, 3854, 3906 },
    { 0, 3907, 3978 },
    { 0, 3979, 4069 },
    { 0, 4070, 4148 },
    { 0, 4149, 4243 },
    { 0, 4244, 4274 },
    { 0, 4275, 4344 },
    { 0, 4345, 4383 },
    { 0, 4384, 4406 },
    { 0, 4407, 4435 },
    { 0, 4436, 4556 },
    { 0, 4557, 4740 },
    { 0, 4741, 4557 },
    { 0, 4558, 4740 },
};
GsLINE gFocusLines[2] = {
    { 0x40000000, 0, 70, 320, 70, 255, 255, 255 },
    { 0x40000000, 103, 0, 103, 240, 255, 255, 255 },
};

char gTrackImage[80];
