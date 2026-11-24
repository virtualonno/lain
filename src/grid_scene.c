/* 3D grid cylinder scene: builds and animates the 8-column by 3-row-per-level layout (900 Y-units
 * per level, 512/4096 yaw per column), camera paths, cursor highlight, and cell-flight animations. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <abs.h>
#include <rand.h>
#include <stdio.h>
#include "nodes.h"
#include "graphics.h"
#include "grid_effects.h"
#include "grid_menu.h"
#include "grid_nodes.h"
#include "grid_scene.h"
#include "lapks_player.h"
#include "clip.h"
#include "pad.h"
#include "sound.h"
#include "text.h"
#include "band_tmd.h"
#include "media_cube_tmd.h"
#include "tmd_data_5.h"
#include "media_cube.h"


unsigned char gRingSettleSoundPending;
int gLevelTravelDistance;
unsigned char gRebuildLevelModels;
short gLevelStep;
int gLevelsToTravel;
int gLevelTravelSpeed;
int gSceneAnimation;
unsigned short gWaitFrames; /* this unit reads gWaitFrames as an unsigned short; src/special_scenes.c declares it as int */

/* The first two words of these embedded TMDs. */
struct TmdIdFlags {
    u_long id;
    u_long flags;
};
extern struct TmdIdFlags gEvenLevelLabelTmd;
extern struct TmdIdFlags gOddLevelLabelTmd;


int IsPointOffScreen(const short *point, short margin)
{
    if (point[0] > -margin && point[0] < 320 + margin &&
        point[1] > -margin && point[1] < 240 + margin) {
        return 0;
    }
    return 1;
}


int gCellModelCount;

extern u_long gPointTmd[2];

/* The embedded TIMs are addressed through their magic and flags words. */
extern u_long gLifLogoTim[2];
extern u_long gLifeInstinctFunctionTim[2];
extern u_long gTextBandBarTim[2];
extern u_long gSiteALevelTim[2];
extern u_long gSiteBLevelTim[2];
extern u_long gLevelDigitsTim[2];
extern u_long gEvenLevelTensTim[2];
extern u_long gEvenLevelOnesTim[2];
extern u_long gOddLevelTensTim[2];
extern u_long gOddLevelOnesTim[2];
extern u_long gLainIconTim[2];
extern u_long gLainDarkIconTim[2];
extern u_long gToukoIconTim[2];
extern u_long gLabIconTim[2];
extern u_long gSaiseikunIconTim[2];
extern u_long gTransferIconTim[2];
extern u_long gMovieIconTim[2];
extern u_long gPsychiatryIconTim[2];
extern u_long gFlyingCellTim;
extern u_long gCursorHighlightTim[2];
extern u_long gOpenedLainIconTim[2];
extern u_long gOpenedLainDarkIconTim[2];
extern u_long gOpenedToukoIconTim[2];
extern u_long gOpenedLabIconTim[2];
extern u_long gOpenedSaiseikunIconTim[2];
extern u_long gOpenedTransferIconTim[2];
extern u_long gOpenedMovieIconTim[2];
extern u_long gOpenedPsychiatryIconTim[2];
enum { HIGHLIGHT_LEVEL, HIGHLIGHT_CELL };

void InitGridScene(void)
{
    unsigned int level, cell;
    int now;

    DrawSync(0);
    VSync(0);
    InitModelCoordinate(&gModels[1]);
    SetModelTmd(gPointTmd, &gModels[1]);
    LinkModelObjects(&gModels[1], 0);
    gModels[1].coord.coord.t[2] = gView.vpz;
    InitModelCoordinate(&gModels[5]);
    SetModelTmd(gPointTmd, &gModels[5]);
    LinkModelObjects(&gModels[5], &gModels[1].coord);
    gCellModelCount = 0;
    gRebuildLevelModels = 1;
    gModels[5].coord.coord.t[2] = -gView.vpz;
    for (level = 0; level < 4; level++) {
        for (cell = 0; cell < 24; cell++) {
            BuildCellModel(level, cell);
        }
        BuildLevelModels(level, level, 0, 0);
    }
    UploadTim(gLifLogoTim);
    UploadTim(gLifeInstinctFunctionTim);
    UploadTim(gTextBandBarTim);
    UploadTim(gLevelDigitsTim);
    UploadTim(gEvenLevelTensTim);
    UploadTim(gEvenLevelOnesTim);
    UploadTim(gOddLevelTensTim);
    UploadTim(gOddLevelOnesTim);
    UploadTim(gLainIconTim);
    UploadTim(gLainDarkIconTim);
    UploadTim(gToukoIconTim);
    UploadTim(gLabIconTim);
    UploadTim(gSaiseikunIconTim);
    UploadTim(gTransferIconTim);
    UploadTim(gMovieIconTim);
    UploadTim(gPsychiatryIconTim);
    UploadTim(&gFlyingCellTim);
    UploadTim(gCursorHighlightTim);
    UploadTim(gOpenedLainIconTim);
    UploadTim(gOpenedLainDarkIconTim);
    UploadTim(gOpenedToukoIconTim);
    UploadTim(gOpenedLabIconTim);
    UploadTim(gOpenedSaiseikunIconTim);
    UploadTim(gOpenedTransferIconTim);
    UploadTim(gOpenedMovieIconTim);
    UploadTim(gOpenedPsychiatryIconTim);
    if (gCurrentDisc == 0) {
        UploadTim(gSiteALevelTim);
    } else {
        UploadTim(gSiteBLevelTim);
    }
    now = gVSyncCount;
    gSceneAnimation = SCENE_CMD_NONE;
    gViewTiltedDown = 0;
    gViewTiltedUp = 0;
    gTiltOffsetY = 0;
    gPointDeltaCount = 0;
    gPointDeltaStepsLeft = 0;
    gViewPathStep = 0.0;
    gLightFading = 0;
    gCellFlying = 0;
    gRingSettleSoundPending = 0;
    gRebuildLevelModels = 0;
    gClipFrameOffsetX = 12;
    gRingRippleDecayAt = now;
    gRingDipRollAt = now;
    InitClipFrameSprites();
    InitHorizonAndStreaks();
    SetLevelNumberTexture(gGridLevel);
    gHighlightSelection[HIGHLIGHT_LEVEL] = 0xff;
    gHighlightSelection[HIGHLIGHT_CELL] = 0xff;
    srand(0xa);
    StartViewDescent();
    SetViewLights();
    InitTexturedQuad();
}

void BuildCellModel(unsigned int level, unsigned int cell)
{
    int yOffset;

    if (level < (unsigned int)GetGridLevelCount() && cell < 24 && ++gCellModelCount < 98U) {
        InitModelCoordinate(&gCellModels[gCellModelCount + 1]);
        if (gGridCells[level][cell].nodeIndex == -1) {
            int slot = gCellModelCount;
            SetModelTmd(gEmptyCellTmd, &gCellModels[slot + 1]);
        } else {
            SetModelTmd(gCellTmds[gGridCells[level][cell].modelType], &gCellModels[gCellModelCount + 1]);
        }
        LinkModelObjects(&gCellModels[gCellModelCount + 1], &gModels[5].coord);
        gCellModels[gCellModelCount + 1].coord.coord.t[0] = gCellPositions[cell].vx;
        yOffset = (int)level * 900 - 100;
        gCellModels[gCellModelCount + 1].coord.coord.t[1] = gCellPositions[cell].vy - yOffset;
        gCellModels[gCellModelCount + 1].coord.coord.t[2] = gCellPositions[cell].vz;
        gCellModels[gCellModelCount + 1].rot.vy = (int)((22.5 + cell * 45 - cell * 360) * 4096 / 360);
        UpdateModelMatrix(&gCellModels[gCellModelCount + 1]);
        gCellModels[gCellModelCount + 1].object.attribute = 0xd0000000;
        gGridCells[level][cell].modelIndex = gCellModelCount;
    }
}

void RebuildCellModel(unsigned int level, unsigned int cell)
{
    int yOffset;

    if (level < (unsigned int)GetGridLevelCount() && cell < 24) {
        gCellModelCount = gGridCells[level][cell].modelIndex;
        InitModelCoordinate(&gCellModels[gCellModelCount + 1]);
        if (gGridCells[level][cell].nodeIndex == -1) {
            int slot = gCellModelCount;
            SetModelTmd(gEmptyCellTmd, &gCellModels[slot + 1]);
        } else {
            SetModelTmd(gCellTmds[gGridCells[level][cell].modelType], &gCellModels[gCellModelCount + 1]);
        }
        LinkModelObjects(&gCellModels[gCellModelCount + 1], &gModels[5].coord);
        gCellModels[gCellModelCount + 1].coord.coord.t[0] = gCellPositions[cell].vx;
        yOffset = (int)level * 900 - 100;
        gCellModels[gCellModelCount + 1].coord.coord.t[1] = gCellPositions[cell].vy - yOffset;
        gCellModels[gCellModelCount + 1].coord.coord.t[2] = gCellPositions[cell].vz;
        gCellModels[gCellModelCount + 1].rot.vy = (int)((22.5 + cell * 45 - cell * 360) * 4096 / 360);
        UpdateModelMatrix(&gCellModels[gCellModelCount + 1]);
        gCellModels[gCellModelCount + 1].object.attribute = 0xd0000000;
        gGridCells[level][cell].modelIndex = gCellModelCount;
    }
}

void BuildLevelModels(int group, int level, int firstYaw, int secondYaw)
{
    InitModelCoordinate(&gLevelModels[group + 1]);
    SetModelTmd(gTextBandTmd, &gLevelModels[group + 1]);
    LinkModelObjects(&gLevelModels[group + 1], &gModels[5].coord);
    gLevelModels[group + 1].coord.coord.t[0] = gTextBandPosition.vx;
    gLevelModels[group + 1].coord.coord.t[1] = gTextBandPosition.vy - level * 900;
    gLevelModels[group + 1].coord.coord.t[2] = gTextBandPosition.vz;
    gLevelModels[group + 1].rot.vy = firstYaw;
    gLevelModels[group + 1].object.attribute = 0x80000000;
    gLevelModels[group + 1].rot.vy = -227;
    UpdateModelMatrix(&gLevelModels[group + 1]);

    InitModelCoordinate(&gLevelModels[group + 5]);
    SetModelTmd(gLabelBandTmd, &gLevelModels[group + 5]);
    LinkModelObjects(&gLevelModels[group + 5], &gModels[5].coord);
    gLevelModels[group + 5].coord.coord.t[0] = gLabelBandPosition.vx;
    gLevelModels[group + 5].coord.coord.t[1] = gLabelBandPosition.vy - level * 900;
    gLevelModels[group + 5].coord.coord.t[2] = gLabelBandPosition.vz;
    gLevelModels[group + 5].rot.vy = secondYaw;
    gLevelModels[group + 5].object.attribute = 0x80000000;

    InitModelCoordinate(&gLevelModels[group + 9]);
    if (level & 1) {
        SetModelTmd(&gOddLevelLabelTmd.id, &gLevelModels[group + 9]);
    } else {
        SetModelTmd(&gEvenLevelLabelTmd.id, &gLevelModels[group + 9]);
    }
    LinkModelObjects(&gLevelModels[group + 9], &gLevelModels[group + 5].coord);
    gLevelModels[group + 9].coord.coord.t[0] = gLevelLabelPositions[0].vx;
    gLevelModels[group + 9].coord.coord.t[1] = gLevelLabelPositions[0].vy;
    gLevelModels[group + 9].coord.coord.t[2] = gLevelLabelPositions[0].vz;
    gLevelModels[group + 9].rot.vy = 0;
    UpdateModelMatrix(&gLevelModels[group + 9]);
    gLevelModels[group + 9].object.attribute = 0x80000000;

    InitModelCoordinate(&gLevelModels[group + 13]);
    if (level & 1) {
        SetModelTmd(&gOddLevelLabelTmd.id, &gLevelModels[group + 13]);
    } else {
        SetModelTmd(&gEvenLevelLabelTmd.id, &gLevelModels[group + 13]);
    }
    LinkModelObjects(&gLevelModels[group + 13], &gLevelModels[group + 5].coord);
    gLevelModels[group + 13].coord.coord.t[0] = gLevelLabelPositions[1].vx;
    gLevelModels[group + 13].coord.coord.t[1] = gLevelLabelPositions[1].vy;
    gLevelModels[group + 13].coord.coord.t[2] = gLevelLabelPositions[1].vz;
    gLevelModels[group + 13].rot.vy = 2048;
    UpdateModelMatrix(&gLevelModels[group + 13]);
    gLevelModels[group + 13].object.attribute = 0x80000000;

    InitModelCoordinate(&gLevelModels[group + 17]);
    SetModelTmd(gTextBandTetrahedronTmd, &gLevelModels[group + 17]);
    LinkModelObjects(&gLevelModels[group + 17], &gModels[5].coord);
    gLevelModels[group + 17].coord.coord.t[0] = gTextBandPosition.vx;
    gLevelModels[group + 17].coord.coord.t[1] = gTextBandPosition.vy - level * 900;
    gLevelModels[group + 17].coord.coord.t[2] = gTextBandPosition.vz;
    gLevelModels[group + 17].object.attribute = 0x80000000;
}

int gViewTiltedUp;
int gViewTiltedDown;

void UpdateGridScene(void)
{
    unsigned int column;

    GetGridLevelCount();
    if (gSceneCommand == SCENE_CMD_NONE && gSceneAnimation != SCENE_CMD_NONE) {
        gSceneAnimation = SCENE_CMD_NONE;
    }
    if (gSceneAnimation == SCENE_CMD_NONE) {
        switch (gSceneCommand) {
        case SCENE_CMD_LEVEL_UP:
            if (gGridLevel < GetGridLevelCount() - 1) {
                gSceneAnimation = SCENE_CMD_LEVEL_UP;
                gGridBusy = 255;
                gRebuildLevelModels = 1;
                gLevelStep = 0;
                gWaitFrames = 0;
                gRingRippleMode = RING_RIPPLE_RESET;
            }
            break;
        case SCENE_CMD_LEVEL_DOWN:
            if (gGridLevel > 0) {
                gSceneAnimation = SCENE_CMD_LEVEL_DOWN;
                gGridBusy = 255;
                gRebuildLevelModels = 1;
                gLevelStep = 0;
                gWaitFrames = 0;
                gRingRippleMode = RING_RIPPLE_RESET;
            }
            break;
        case SCENE_CMD_TURN_LEFT:
            gSceneAnimation = SCENE_CMD_TURN_LEFT;
            gWaitFrames = 0;
            gTurnStep = 0;
            gGridBusy = 255;
            break;
        case SCENE_CMD_TURN_RIGHT:
            gSceneAnimation = SCENE_CMD_TURN_RIGHT;
            gWaitFrames = 0;
            gTurnStep = 0;
            gGridBusy = 255;
            break;
        case SCENE_CMD_TILT_UP:
            if (gViewTiltedUp == 0) {
                gSceneAnimation = SCENE_CMD_TILT_UP;
                gTurnStep = 0;
                gGridBusy = 255;
                if (gViewTiltedDown == 0) {
                    gViewTiltedUp = 1;
                }
            }
            break;
        case SCENE_CMD_TILT_DOWN:
            if (gViewTiltedDown == 0) {
                gSceneAnimation = SCENE_CMD_TILT_DOWN;
                gTurnStep = 0;
                gGridBusy = 255;
                if (gViewTiltedUp == 0) {
                    gViewTiltedDown = 1;
                }
            }
            break;
        case SCENE_CMD_TRAVEL_TO_CURSOR:
            gSceneAnimation = SCENE_CMD_TRAVEL_TO_CURSOR;
            gGridBusy = 255;
            if (gCursorLevel >= GetGridLevelCount()) {
                gCursorLevel = GetGridLevelCount() - 1;
            }
            if (gCursorCell >= 24) {
                gCursorCell = 23;
            }
            gLevelsToTravel = gCursorLevel - gGridLevel;
            column = gCursorCell;
            while (column >= 9) {
                column -= 8;
            }
            gLevelTravelDistance = 0;
            gColumnsToTurn = column - gGridHeading;
            gModels[7].coord.coord.t[1] = gRingRestPosition.vy + 900;
            UpdateModelMatrix(&gModels[7]);
            break;
        case SCENE_CMD_LEVEL_TRAVEL:
            gSceneAnimation = SCENE_CMD_LEVEL_TRAVEL;
            gGridBusy = 255;
            if (gCursorLevel >= GetGridLevelCount()) {
                gCursorLevel = GetGridLevelCount() - 1;
            }
            if (gCursorCell >= 24) {
                gCursorCell = 23;
            }
            gLevelsToTravel = gCursorLevel - gGridLevel;
            column = gCursorCell;
            while (column >= 9) {
                column -= 8;
            }
            gColumnsToTurn = column - gGridHeading;
            if (gLevelsToTravel > 0) {
                gLevelTravelSpeed = abs(gLevelsToTravel) * 100;
            } else {
                gLevelTravelSpeed = abs(gLevelsToTravel) * 75;
            }
            gLevelTravelDistance = 0;
            gLevelStep = 0;
            gWaitFrames = 0;
            gRingRippleMode = RING_RIPPLE_RESET;
            break;
        case SCENE_CMD_VIEW_TO_TOP:
            StartViewPath(VIEW_TOP);
            gSceneAnimation = SCENE_CMD_VIEW_TO_TOP;
            break;
        case SCENE_CMD_VIEW_TO_FRONT:
            StartViewPath(VIEW_FRONT_RETURN);
            gSceneAnimation = SCENE_CMD_VIEW_TO_FRONT;
            break;
        case SCENE_CMD_CELL_FLIGHT:
            StartCellFlight();
            gSceneAnimation = SCENE_CMD_CELL_FLIGHT;
            break;
        case SCENE_CMD_RING_BURST:
            StepRing();
            InitPieceBurst();
            gSceneAnimation = SCENE_CMD_RING_BURST;
            break;
        case SCENE_CMD_QUAD_EXPAND:
            DrawOrbitingTexturedQuad();
            StartExpandingTexturedQuad();
            gSceneAnimation = SCENE_CMD_QUAD_EXPAND;
            break;
        case SCENE_CMD_FLASH_CLIP_FRAME:
            gSceneAnimation = SCENE_CMD_FLASH_CLIP_FRAME;
            gGridBusy = 255;
            gFlashFramesLeft = 12;
            gFlashLevel = 255;
            break;
        }
    } else if (gSceneCommand != SCENE_CMD_NONE) {
        switch (gSceneAnimation) {
        case SCENE_CMD_LEVEL_UP:
            if (gWaitFrames >= 12) {
                if (gLevelStep < 9) {
                    if (gLevelStep == 0) {
                        PlaySound(8);
                    } else if (gLevelStep == 2) {
                        PlaySound(9);
                    }
                    if (gLevelStep == 0) {
                        StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_DOWN, 2);
                    }
                    SetLevelNumberTexture(gGridLevel + 1);
                    gRingMotion = RING_MOTION_LEVEL_UP;
                    gModels[5].coord.coord.t[1] += 100;
                    UpdateModelMatrix(&gModels[5]);
                    gLevelStep++;
                } else {
                    gRingMotion = RING_MOTION_RISE_OVERSHOOT;
                    gLevelStep = 0;
                    gWaitFrames = 0;
                    gSceneAnimation = SCENE_CMD_NONE;
                    gSceneCommand = SCENE_CMD_NONE;
                    gGridBusy = 0;
                    gGridLevel++;
                    if (gGridLevel >= GetGridLevelCount()) {
                        gGridLevel = GetGridLevelCount() - 1;
                    }
                    StartPointMorph(gRingPoints, POINT_DELTA_RING_FLAT, 11);
                    gRingSettleSoundPending = 1;
                    gRebuildLevelModels = 1;
                }
            } else {
                if (gWaitFrames == 1) {
                    StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_UP, 11);
                }
                gWaitFrames++;
            }
            break;
        case SCENE_CMD_LEVEL_DOWN:
            if (gWaitFrames >= 13) {
                if (gLevelStep < 12) {
                    if (gLevelStep == 0) {
                        PlaySound(8);
                    } else if (gLevelStep == 2) {
                        PlaySound(9);
                    }
                    if (gLevelStep == 0) {
                        StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_UP, 12);
                    }
                    SetLevelNumberTexture(gGridLevel - 1);
                    gRingMotion = RING_MOTION_LEVEL_DOWN;
                    gModels[5].coord.coord.t[1] -= 75;
                    UpdateModelMatrix(&gModels[5]);
                    gLevelStep++;
                } else {
                    PlaySound(10);
                    gRingMotion = RING_MOTION_DROP_OVERSHOOT;
                    gLevelStep = 0;
                    gWaitFrames = 0;
                    gSceneAnimation = SCENE_CMD_NONE;
                    gSceneCommand = SCENE_CMD_NONE;
                    gGridBusy = 0;
                    gGridLevel--;
                    if (gGridLevel < 0) {
                        gGridLevel = 0;
                    } else {
                        SetLevelNumberTexture(gGridLevel - 1);
                    }
                    StartPointMorph(gRingPoints, POINT_DELTA_RING_FLAT, 12);
                    gRebuildLevelModels = 1;
                }
            } else {
                gWaitFrames++;
            }
            break;
        case SCENE_CMD_TURN_LEFT:
            if (gWaitFrames >= 10) {
                if (gTurnStep < 25) {
                    if (gTurnStep == 0 || gTurnStep == 7 || gTurnStep == 14 || gTurnStep == 21) {
                        PlaySound(11);
                    }
                    gRingMotion = RING_MOTION_TURN_LEFT;
                    gModels[5].rot.vy -= 19.342222222222222;
                    UpdateModelMatrix(&gModels[5]);
                    gTurnStep++;
                } else {
                    gRingMotion = RING_MOTION_RETURN_FROM_LEFT;
                    gTurnStep = 0;
                    gWaitFrames = 0;
                    gGridBusy = 0;
                    gSceneAnimation = SCENE_CMD_NONE;
                    gSceneCommand = SCENE_CMD_NONE;
                    if (++gGridHeading >= 8) {
                        gGridHeading = 0;
                    }
                    gModels[5].rot.vy = gHeadingYawEighths[gGridHeading] * 512;
                    UpdateModelMatrix(&gModels[5]);
                }
            } else {
                gWaitFrames++;
            }
            break;
        case SCENE_CMD_TURN_RIGHT:
            if (gWaitFrames >= 10) {
                if (gTurnStep < 26) {
                    if (gTurnStep == 0 || gTurnStep == 7 || gTurnStep == 14 || gTurnStep == 21) {
                        PlaySound(11);
                    }
                    gRingMotion = RING_MOTION_TURN_RIGHT;
                    gModels[5].rot.vy += 19.342222222222222;
                    UpdateModelMatrix(&gModels[5]);
                    gTurnStep++;
                } else {
                    gRingMotion = RING_MOTION_RETURN_FROM_RIGHT;
                    gTurnStep = 0;
                    gWaitFrames = 0;
                    gGridBusy = 0;
                    gSceneAnimation = SCENE_CMD_NONE;
                    gSceneCommand = SCENE_CMD_NONE;
                    if (--gGridHeading < 0) {
                        gGridHeading = 7;
                    }
                    gModels[5].rot.vy = gHeadingYawEighths[gGridHeading] * 512;
                    UpdateModelMatrix(&gModels[5]);
                }
            } else {
                gWaitFrames++;
            }
            break;
        case SCENE_CMD_TILT_UP:
            if (gTurnStep < 6) {
                gModels[1].rot.vx -= 34;
                UpdateModelMatrix(&gModels[1]);
                gTurnStep += 3;
                gTiltOffsetY += 21;
            } else {
                gTurnStep = 0;
                gGridBusy = 0;
                gViewTiltedDown = 0;
                gSceneAnimation = SCENE_CMD_NONE;
                gSceneCommand = SCENE_CMD_NONE;
            }
            break;
        case SCENE_CMD_TILT_DOWN:
            if (gTurnStep < 6) {
                gModels[1].rot.vx += 34;
                UpdateModelMatrix(&gModels[1]);
                gTurnStep += 3;
                gTiltOffsetY -= 21;
            } else {
                gTurnStep = 0;
                gGridBusy = 0;
                gViewTiltedUp = 0;
                gSceneAnimation = SCENE_CMD_NONE;
                gSceneCommand = SCENE_CMD_NONE;
            }
            break;
        case SCENE_CMD_TRAVEL_TO_CURSOR:
            StepOrFinishTravelToCursor();
            break;
        case SCENE_CMD_LEVEL_TRAVEL:
            StepLevelTravel();
            break;
        case SCENE_CMD_VIEW_TO_TOP:
        case SCENE_CMD_VIEW_TO_FRONT:
            StepViewPath();
            break;
        case SCENE_CMD_CELL_FLIGHT:
            if (gFlightEnding == 0) {
                StepCellFlight();
            } else {
                StepCellFlightEnd();
            }
            break;
        case SCENE_CMD_RING_BURST:
            StepPieceBurst();
            break;
        case SCENE_CMD_QUAD_EXPAND:
            DrawExpandingTexturedQuad();
            break;
        case SCENE_CMD_FLASH_CLIP_FRAME:
            StepClipFrameFlash();
            break;
        default:
            gSceneAnimation = SCENE_CMD_NONE;
            break;
        }
    }
    StepViewDescent();
    StepLightFade();
    SortGridScene();
    ProjectCellsToScreen();
    DrawOrbitingTexturedQuad();
    if (gViewMode == VIEW_FRONT && gSceneAnimation != SCENE_CMD_RING_BURST) {
        StepRing();
    }
}

void StepLevelTravel(void)
{
    if (gLevelsToTravel > 0) {
        if (gWaitFrames >= 12) {
            if (gLevelStep == 0) {
                PlaySound(8);
            } else if (gLevelStep == 2) {
                PlaySound(9);
            }
            if (gLevelStep == 0) {
                StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_DOWN, 2);
            }
            gModels[5].coord.coord.t[1] += gLevelTravelSpeed;
            gLevelTravelDistance += gLevelTravelSpeed;
            UpdateModelMatrix(&gModels[5]);
            gRingMotion = RING_MOTION_LEVEL_UP;
            gLevelStep++;
            if (gLevelTravelDistance >= 900) {
                SetLevelNumberTexture(gGridLevel + 1);
                gRebuildLevelModels = 1;
                gLevelTravelDistance -= 900;
                gGridLevel++;
                gLevelsToTravel--;
                if (gGridLevel >= GetGridLevelCount()) {
                    gGridLevel = GetGridLevelCount() - 1;
                    gLevelsToTravel = 0;
                }
            }
        } else {
            if (gWaitFrames == 1) {
                StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_UP, 11);
            }
            gWaitFrames++;
        }
        if (gLevelsToTravel == 0) {
            gRingMotion = RING_MOTION_RISE_OVERSHOOT;
            StartPointMorph(gRingPoints, POINT_DELTA_RING_FLAT, 11);
            gRingSettleSoundPending = 1;
        }
    } else if (gLevelsToTravel < 0) {
        if (gWaitFrames >= 13) {
            if (gLevelStep == 0) {
                PlaySound(8);
            } else if (gLevelStep == 2) {
                PlaySound(9);
            }
            if (gLevelStep == 0) {
                StartPointMorph(gRingPoints, POINT_DELTA_RING_BENT_UP, 12);
            }
            gModels[5].coord.coord.t[1] -= gLevelTravelSpeed;
            gLevelTravelDistance += gLevelTravelSpeed;
            UpdateModelMatrix(&gModels[5]);
            gRingMotion = RING_MOTION_LEVEL_DOWN;
            gLevelStep++;
            if (gLevelTravelDistance >= 900) {
                gLevelTravelDistance -= 900;
                gRebuildLevelModels = 1;
                gGridLevel--;
                gLevelsToTravel++;
                if (gGridLevel < 0) {
                    gGridLevel = 0;
                    gLevelsToTravel = 0;
                } else {
                    SetLevelNumberTexture(gGridLevel - 1);
                }
            }
        } else {
            gWaitFrames++;
        }
        if (gLevelsToTravel == 0) {
            PlaySound(10);
            gRingMotion = RING_MOTION_DROP_OVERSHOOT;
            StartPointMorph(gRingPoints, POINT_DELTA_RING_FLAT, 12);
        }
    } else {
        gWaitFrames = 0;
        gLevelStep = 0;
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
    }
}

short gTurnStep;
int gColumnsToTurn;

void StepTravelToCursor(void)
{
    if (gLevelsToTravel > 0) {
        gModels[5].coord.coord.t[1] += 100;
        gLevelTravelDistance += 100;
        UpdateModelMatrix(&gModels[5]);
        SetLevelNumberTexture(gGridLevel + 1);
        if (gLevelTravelDistance >= 900) {
            gLevelTravelDistance -= 900;
            gRebuildLevelModels = 1;
            gGridLevel++;
            gLevelsToTravel--;
            if (gGridLevel >= GetGridLevelCount()) {
                gGridLevel = GetGridLevelCount() - 1;
                gLevelsToTravel = 0;
            }
        }
    } else if (gLevelsToTravel < 0) {
        gModels[5].coord.coord.t[1] -= 100;
        gLevelTravelDistance += 100;
        UpdateModelMatrix(&gModels[5]);
        if (gLevelTravelDistance >= 900) {
            gLevelTravelDistance -= 900;
            gRebuildLevelModels = 1;
            gGridLevel--;
            gLevelsToTravel++;
            if (gGridLevel < 0) {
                gGridLevel = 0;
                gLevelsToTravel = 0;
            } else {
                SetLevelNumberTexture(gGridLevel - 1);
            }
        }
    } else if (gColumnsToTurn > 0) {
        gModels[5].rot.vy = gModels[5].rot.vy - 19.342222222222222;
        UpdateModelMatrix(&gModels[5]);
        if (++gTurnStep >= 25) {
            gTurnStep = 0;
            gGridHeading++;
            gColumnsToTurn--;
            if (gGridHeading >= 8) {
                gGridHeading = 0;
            }
            gModels[5].rot.vy = gHeadingYawEighths[gGridHeading] * 512;
            UpdateModelMatrix(&gModels[5]);
        }
    } else if (gColumnsToTurn < 0) {
        gModels[5].rot.vy = gModels[5].rot.vy + 19.342222222222222;
        UpdateModelMatrix(&gModels[5]);
        if (++gTurnStep >= 26) {
            gTurnStep = 0;
            gGridHeading--;
            gColumnsToTurn++;
            if (gGridHeading < 0) {
                gGridHeading = 7;
            }
            gModels[5].rot.vy = gHeadingYawEighths[gGridHeading] * 512;
            UpdateModelMatrix(&gModels[5]);
        }
    } else {
        gRingMotion = RING_MOTION_RISE_OVERSHOOT;
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
    }
}

short gClipFrameOffsetX;
short gTiltOffsetY;

void DrawClipFrame(void)
{
    gClipFrameSprites[0].x = gClipFrameOffsetX + (148 - gClipFrameInfo.x);
    gClipFrameSprites[0].y = gTiltOffsetY + (304 - gClipFrameInfo.y);
    if (gClipFrameInfo.width > 256) {
        gClipFrameSprites[0].w = 256;
    } else {
        gClipFrameSprites[0].w = gClipFrameInfo.width;
    }
    if (gClipFrameInfo.height > 256) {
        gClipFrameSprites[0].h = 256;
    } else {
        gClipFrameSprites[0].h = gClipFrameInfo.height;
    }
    GsSortSprite(&gClipFrameSprites[0], &gOt[gActiveBuffer], 350);
    DrawSync(0);

    gClipFrameSprites[1].w = 64;
    gClipFrameSprites[1].x = gClipFrameSprites[0].x + gClipFrameSprites[0].w;
    gClipFrameSprites[1].y = gTiltOffsetY + (304 - gClipFrameInfo.y);
    gClipFrameSprites[1].h = gClipFrameSprites[0].h;
    GsSortSprite(&gClipFrameSprites[1], &gOt[gActiveBuffer], 350);
    DrawSync(0);

    gClipFrameSprites[2].x = gClipFrameOffsetX + (148 - gClipFrameInfo.x);
    gClipFrameSprites[2].y = gClipFrameSprites[0].y + gClipFrameSprites[0].h;
    gClipFrameSprites[2].w = gClipFrameSprites[0].w;
    gClipFrameSprites[2].h = 480 - gClipFrameSprites[0].h;
    GsSortSprite(&gClipFrameSprites[2], &gOt[gActiveBuffer], 350);
    DrawSync(0);

    gClipFrameSprites[3].w = 64;
    gClipFrameSprites[3].x = gClipFrameSprites[0].x + gClipFrameSprites[0].w;
    gClipFrameSprites[3].y = gClipFrameSprites[0].y + gClipFrameSprites[0].h;
    gClipFrameSprites[3].h = 480 - gClipFrameSprites[0].h;
    GsSortSprite(&gClipFrameSprites[3], &gOt[gActiveBuffer], 350);
    DrawSync(0);
}

extern u_long gFlyingCellTimFlags[];

RotatedCoordinatePair gFlightLines[20];
RotatedCoordinate gFlightEndCoords[20]; /* 1760 bytes; StepCellFlightEnd sets their translations, no routine reads them */
SVECTOR gFlyingCellPoints[8];
SVECTOR gSavedPointDeltas[256];

u_char gCellSplit;
u_char gCellFlying;
short gCellSplitX;
short gCellSplitY;
short gCellSplitZ;
int gFlightEndFramesLeft;
u_char gFlightEnding;
int gFlightClipSet;
int gFlightEndStep;
int gSavedPointDeltaMode;
int gFlightLevel;
int gFlightCell;
int gSavedPointDeltaStepsLeft;

u_char gDescentPathDone;
double gViewPathStep;

short gFlashLevel;
short gFlashFramesLeft;

void StepClipFrameFlash(void)
{
    u_char color;
    short nextColor;

    if (gFlashFramesLeft != 0) {
        gFlashFramesLeft--;
        nextColor = gFlashLevel - 10;
        color = gFlashLevel;
        gClipFrameSprites[0].b = color;
        gClipFrameSprites[0].g = color;
        gClipFrameSprites[0].r = color;
        gClipFrameSprites[1].b = color;
        gClipFrameSprites[1].g = color;
        gClipFrameSprites[1].r = color;
        gClipFrameSprites[2].b = color;
        gClipFrameSprites[2].g = color;
        gClipFrameSprites[2].r = color;
        gClipFrameSprites[3].b = color;
        gClipFrameSprites[3].g = color;
        gClipFrameSprites[3].r = color;
        gFlashLevel = nextColor;
    } else if (gTiltOffsetY <= 0) {
        gClipFrameSprites[0].b = 128;
        gClipFrameSprites[0].g = 128;
        gClipFrameSprites[0].r = 128;
        gClipFrameSprites[1].b = 128;
        gClipFrameSprites[1].g = 128;
        gClipFrameSprites[1].r = 128;
        gClipFrameSprites[2].b = 128;
        gClipFrameSprites[2].g = 128;
        gClipFrameSprites[2].r = 128;
        gClipFrameSprites[3].b = 128;
        gClipFrameSprites[3].g = 128;
        gClipFrameSprites[3].r = 128;
        gFlashFramesLeft = 0;
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
    }
}

void ProjectCellsToScreen(void)
{
    DVECTOR screen;
    int levelCount;
    int level;
    unsigned int group;
    unsigned int i;

    levelCount = GetGridLevelCount();
    level = gGridLevel;
    if (level == levelCount - 1) {
        level = levelCount - 3;
    } else if (level > 0) {
        level--;
    }
    for (group = 0; group < 3; group++) {
        for (i = 0; i < 24; i++) {
            ProjectModelToScreen(&gCellModels[gGridCells[level][i].modelIndex + 1], &screen);
            gGridCells[level][i].screenX = screen.vx;
            gGridCells[level][i].screenY = screen.vy;
        }
        level++;
    }
}

extern u_long gLevelDigitsTimFlags[];
extern u_long gEvenLevelTensTimFlags[];
extern u_long gEvenLevelOnesTimFlags[];
extern u_long gOddLevelTensTimFlags;
extern u_long gOddLevelOnesTimFlags[];
char gLevelNumberFormat[8] = "%02d";

void SortGridScene(void)
{
    int levelCount = GetGridLevelCount();
    int currentLevel = gGridLevel;
    int level;
    unsigned int i, cell;
    DVECTOR rotation[3];
    int group;
    int slot;

    if (currentLevel >= levelCount - 1) {
        level = currentLevel - 2;
    } else {
        level = currentLevel;
        if (level > 0) {
            --level;
        }
    }
    group = 0;
    for (i = 0; i < 3; i++) {
        rotation[i].vx = gLevelModels[i + 1].rot.vy;
        rotation[i].vy = gLevelModels[i + 5].rot.vy;
    }
    if (gRebuildLevelModels == 1) {
        gCellModelCount = 0;
    }
    if (level != 0) {
        if (gSceneAnimation == SCENE_CMD_LEVEL_DOWN) {
            for (cell = 0; cell < 24; cell++) {
                if (gRebuildLevelModels == 1) {
                    BuildCellModel(level - 1, cell);
                }
                if (gGridCells[level - 1][cell].nodeIndex != -1) {
                    slot = gGridCells[level - 1][cell].modelIndex;
                    if (gCellFlying == 0 || gFlightLevel != level || gFlightCell != cell) {
                        gCellModels[slot + 1].object.attribute &= 0x7fffffff;
                        SortModel(&gCellModels[slot + 1], gOt[gActiveBuffer]);
                    } else {
                        gCellModels[slot + 1].object.attribute |= 0x80000000;
                    }
                }
            }
            if (gRebuildLevelModels == 1) {
                BuildLevelModels(group, level - 1, 0, 0);
            }
            gLevelModels[group + 1].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 1], gOt[gActiveBuffer]);
            gLevelModels[group + 5].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 5], gOt[gActiveBuffer]);
            gLevelModels[group + 9].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 9], gOt[gActiveBuffer]);
            gLevelModels[group + 13].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 13], gOt[gActiveBuffer]);
            gLevelModels[group + 17].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 17], gOt[gActiveBuffer]);
            group++;
        } else {
            for (cell = 0; cell < 24; cell++) {
                if (gGridCells[level - 1][cell].nodeIndex != -1) {
                    slot = gGridCells[level - 1][cell].modelIndex;
                    gCellModels[slot + 1].object.attribute |= 0x80000000;
                }
            }
        }
    }
    for (i = 0; i < 3; i++) {
        for (cell = 0; cell < 24; cell++) {
            if (gRebuildLevelModels == 1) {
                BuildCellModel(level, cell);
            }
            if (gGridCells[level][cell].nodeIndex != -1) {
                slot = gGridCells[level][cell].modelIndex;
                if (gCellFlying == 0 || gFlightLevel != level || gFlightCell != cell) {
                    if (level == gCursorLevel && cell == gCursorCell && gCaptionMoving == 0) {
                        SortCursorHighlight();
                    } else {
                        gCellModels[slot + 1].object.attribute &= 0x7fffffff;
                        SortModel(&gCellModels[slot + 1], gOt[gActiveBuffer]);
                    }
                } else {
                    gCellModels[slot + 1].object.attribute |= 0x80000000;
                }
            }
        }
        if (gRebuildLevelModels == 1) {
            BuildLevelModels(group, level, rotation[i].vx, rotation[i].vy);
        }
        gLevelModels[group + 1].object.attribute &= 0x7fffffff;
        SortModel(&gLevelModels[group + 1], gOt[gActiveBuffer]);
        gLevelModels[group + 5].rot.vy -= 22;
        UpdateModelMatrix(&gLevelModels[group + 5]);
        gLevelModels[group + 5].object.attribute &= 0x7fffffff;
        SortModel(&gLevelModels[group + 5], gOt[gActiveBuffer]);
        gLevelModels[group + 9].object.attribute &= 0x7fffffff;
        SortModel(&gLevelModels[group + 9], gOt[gActiveBuffer]);
        gLevelModels[group + 13].object.attribute &= 0x7fffffff;
        SortModel(&gLevelModels[group + 13], gOt[gActiveBuffer]);
        gLevelModels[group + 17].object.attribute &= 0x7fffffff;
        SortModel(&gLevelModels[group + 17], gOt[gActiveBuffer]);
        group++;
        level++;
    }
    if ((unsigned int)level < (unsigned int)GetGridLevelCount()) {
        if (gSceneAnimation == SCENE_CMD_LEVEL_UP) {
            for (cell = 0; cell < 24; cell++) {
                if (gRebuildLevelModels == 1) {
                    BuildCellModel(level, cell);
                }
                if (gGridCells[level][cell].nodeIndex != -1) {
                    slot = gGridCells[level][cell].modelIndex;
                    if (gCellFlying == 0 || gFlightLevel != level || gFlightCell != cell) {
                        gCellModels[slot + 1].object.attribute &= 0x7fffffff;
                        SortModel(&gCellModels[slot + 1], gOt[gActiveBuffer]);
                    } else {
                        gCellModels[slot + 1].object.attribute |= 0x80000000;
                    }
                }
            }
            if (gRebuildLevelModels == 1) {
                BuildLevelModels(group, level, 0, 0);
            }
            gLevelModels[group + 1].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 1], gOt[gActiveBuffer]);
            gLevelModels[group + 5].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 5], gOt[gActiveBuffer]);
            gLevelModels[group + 9].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 9], gOt[gActiveBuffer]);
            gLevelModels[group + 13].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 13], gOt[gActiveBuffer]);
            gLevelModels[group + 17].object.attribute &= 0x7fffffff;
            SortModel(&gLevelModels[group + 17], gOt[gActiveBuffer]);
        } else {
            for (cell = 0; cell < 24; cell++) {
                if (gGridCells[level][cell].nodeIndex != -1) {
                    slot = gGridCells[level][cell].modelIndex;
                    gCellModels[slot + 1].object.attribute |= 0x80000000;
                }
            }
        }
    }
    if (gCellFlying != 0 || gFlightEnding != 0) {
        SortModel(&gCellModels[99], gOt[gActiveBuffer]);
        if (gCellSplit != 0) {
            SortModel(&gCellModels[100], gOt[gActiveBuffer]);
            SortModel(&gCellModels[101], gOt[gActiveBuffer]);
            SortModel(&gCellModels[102], gOt[gActiveBuffer]);
        }
    }
    gRebuildLevelModels = 0;
}

void SetLevelNumberTexture(int index)
{
    char digitsText[8];
    RECT glyphRect;
    RECT unusedRect; /* 8 bytes; nothing uses it */
    RECT *glyph;
    GsIMAGE digitAtlas;
    GsIMAGE targetTim;
    GsIMAGE unusedTim; /* 28 bytes; nothing uses it */
    unsigned char digit;

    sprintf(digitsText, gLevelNumberFormat, index + 1);
    GsGetTimInfo(gLevelDigitsTimFlags, &digitAtlas);
    if (index & 1) {
        GsGetTimInfo(gOddLevelOnesTimFlags, &targetTim);
        glyph = &glyphRect;
        digit = digitsText[1] - '0';
        glyphRect.x = digitAtlas.px + digit * targetTim.pw;
        glyphRect.y = digitAtlas.py;
        glyphRect.w = targetTim.pw;
        glyphRect.h = targetTim.ph;
        MoveImage(glyph, targetTim.px, targetTim.py);
        DrawSync(0);
        GsGetTimInfo(&gOddLevelTensTimFlags, &targetTim);
    } else {
        GsGetTimInfo(gEvenLevelOnesTimFlags, &targetTim);
        glyph = &glyphRect;
        digit = digitsText[1] - '0';
        glyphRect.x = digitAtlas.px + digit * targetTim.pw;
        glyphRect.y = digitAtlas.py;
        glyphRect.w = targetTim.pw;
        glyphRect.h = targetTim.ph;
        MoveImage(glyph, targetTim.px, targetTim.py);
        DrawSync(0);
        GsGetTimInfo(gEvenLevelTensTimFlags, &targetTim);
    }
    digit = digitsText[0] - '0';
    glyphRect.x = digitAtlas.px + digit * targetTim.pw;
    glyphRect.y = digitAtlas.py;
    glyphRect.w = targetTim.pw;
    glyphRect.h = targetTim.ph;
    MoveImage(glyph, targetTim.px, targetTim.py);
    DrawSync(0);
}

void StartViewPath(u_int mode)
{
    switch (mode) {
    case VIEW_TOP:
        if (gViewMode != VIEW_TOP) {
            gBezierPoints[0][0] = gFrontView.vpx;
            gBezierPoints[0][1] = gFrontView.vpy;
            gBezierPoints[0][2] = gFrontView.vpz;
            gBezierPoints[1][0] = gFrontTopPathPoint.vpx;
            gBezierPoints[1][1] = gFrontTopPathPoint.vpy;
            gBezierPoints[1][2] = gFrontTopPathPoint.vpz;
            gBezierPoints[2][0] = gTopView.vpx;
            gBezierPoints[2][1] = gTopView.vpy;
            gBezierPoints[2][2] = gTopView.vpz;
            WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);
            gGridBusy = 0xff;
            gViewMode = VIEW_TOP;
        } else {
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gGridBusy = 0;
        }
        break;
    case VIEW_FRONT_RETURN:
        if (gViewMode != VIEW_FRONT_RETURN) {
            gBezierPoints[0][0] = gTopView.vpx;
            gBezierPoints[0][1] = gTopView.vpy;
            gBezierPoints[0][2] = gTopView.vpz;
            gBezierPoints[1][0] = gFrontTopPathPoint.vpx;
            gBezierPoints[1][1] = gFrontTopPathPoint.vpy;
            gBezierPoints[1][2] = gFrontTopPathPoint.vpz;
            gBezierPoints[2][0] = gFrontView.vpx;
            gBezierPoints[2][1] = gFrontView.vpy;
            gBezierPoints[2][2] = gFrontView.vpz;
            WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);
            gViewPathStep = 0.0;
            gGridBusy = 0xff;
            gViewMode = VIEW_FRONT_RETURN;
        } else {
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gGridBusy = 0;
        }
        break;
    case VIEW_DESCENT:
        if (gViewMode == VIEW_DESCENT) {
            gBezierPoints[0][0] = gHighTopView.vpx;
            gBezierPoints[0][1] = gHighTopView.vpy;
            gBezierPoints[0][2] = gHighTopView.vpz;
            gBezierPoints[1][0] = gHighFrontPathPoint.vpx;
            gBezierPoints[1][1] = gHighFrontPathPoint.vpy;
            gBezierPoints[1][2] = gHighFrontPathPoint.vpz;
            gBezierPoints[2][0] = gFrontView.vpx;
            gBezierPoints[2][1] = gFrontView.vpy;
            gBezierPoints[2][2] = gFrontView.vpz;
            WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);
            gViewPathStep = 0.0;
            gGridBusy = 0xff;
            gDescentPathDone = 0;
            gViewMode = VIEW_DESCENT;
        } else {
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gGridBusy = 0;
        }
        break;
    }
    gViewPathStep = 0.0;
}

double gFlightStep;

void StepViewPath(void)
{
    double position[3];

    if (gViewPathStep <= 50.0) {
        EvalBezier(3, gWeightedBezierPoints, position, gViewPathStep / 50.0);
        gView.vpx = position[0];
        gView.vpy = position[1];
        gView.vpz = position[2];
        GsSetProjection(350);
        GsSetRefView2(&gView);
        gViewPathStep += 1.0;
    } else {
        gViewPathStep = 0.0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
        SetViewLights();
        if (gViewMode == VIEW_FRONT_RETURN) {
            InitRing();
            gViewMode = VIEW_FRONT;
        }
        if (gViewMode != VIEW_DESCENT) {
            gGridBusy = 0;
        } else {
            gDescentPathDone = 255;
        }
    }
}

extern u_long gCursorHighlightTimFlags[];
int gHighlightSelection[2];

void SortCursorHighlight(void)
{
    GsIMAGE image;
    RECT pixels;
    RECT clut; /* 8 bytes; nothing uses it */
    int oldSelection[2];
    unsigned int cell;
    int slot;

    oldSelection[HIGHLIGHT_LEVEL] = gHighlightSelection[HIGHLIGHT_LEVEL];
    oldSelection[HIGHLIGHT_CELL] = gHighlightSelection[HIGHLIGHT_CELL];
    gHighlightSelection[HIGHLIGHT_LEVEL] = gCursorLevel;
    gHighlightSelection[HIGHLIGHT_CELL] = gCursorCell;
    if (oldSelection[HIGHLIGHT_LEVEL] == gHighlightSelection[HIGHLIGHT_LEVEL] &&
        oldSelection[HIGHLIGHT_CELL] == gHighlightSelection[HIGHLIGHT_CELL]) {
        StepClutFade(gCursorHighlightTim, gCellTims[gGridCells[oldSelection[HIGHLIGHT_LEVEL]][oldSelection[HIGHLIGHT_CELL]].modelType]);
    } else {
        UploadTim(gCursorHighlightTim);
    }
    if (*(u_short *)&gClutFading == 0) {
        UploadTim(gCursorHighlightTim);
    }
    if (gHighlightSelection[HIGHLIGHT_LEVEL] != gCursorLevel || gHighlightSelection[HIGHLIGHT_CELL] != gCursorCell) {
        UploadTim(gCursorHighlightTim);
    }

    slot = gGridCells[gHighlightSelection[HIGHLIGHT_LEVEL]][gHighlightSelection[HIGHLIGHT_CELL]].modelIndex;
    gCellModels[slot + 1].object.attribute |= 0x80000000;
    InitModelCoordinate(&gCellModels[98]);
    SetModelTmd(gCursorHighlightTmd, &gCellModels[98]);
    LinkModelObjects(&gCellModels[98], &gModels[5].coord);

    cell = gHighlightSelection[HIGHLIGHT_CELL];
    gCellModels[98].coord.coord.t[0] = gCellPositions[cell].vx;
    gCellModels[98].coord.coord.t[1] = gCellPositions[cell].vy + 100 - gHighlightSelection[HIGHLIGHT_LEVEL] * 900;
    gCellModels[98].coord.coord.t[2] = gCellPositions[cell].vz;
    gCellModels[98].rot.vy += (cell * 45 + 22.5 - (unsigned int)gHighlightSelection[HIGHLIGHT_CELL] * 360) * 4096 / 360;
    gCellModels[98].object.attribute = 0x50000000;
    UpdateModelMatrix(&gCellModels[98]);

    GsGetTimInfo(gCellTims[gGridCells[gHighlightSelection[HIGHLIGHT_LEVEL]][gHighlightSelection[HIGHLIGHT_CELL]].modelType] + 1, &image);
    pixels.x = image.px;
    pixels.y = image.py;
    pixels.w = image.pw;
    pixels.h = image.ph;
    GsGetTimInfo(gCursorHighlightTimFlags, &image);
    MoveImage(&pixels, image.px, image.py);
    DrawSync(0);
    SortModel(&gCellModels[98], gOt[gActiveBuffer]);
}

void StartCellFlight(void)
{
    GsIMAGE image;
    RECT pixels;
    RECT clut;
    MATRIX localWorld;
    MATRIX localScreen;
    MATRIX rotation = GsIDMATRIX;
    SVECTOR input;
    SVECTOR angle;
    SVECTOR unusedAngle; /* 8 bytes; nothing uses it */
    VECTOR point;
    VECTOR unusedPoint; /* 16 bytes; nothing uses it */
    struct TMD_STRUCT info;
    GridCell (*cells)[24] = gGridCells;
    unsigned int cell;
    int selectedModel;
    unsigned int i;

    gFlightLevel = gCursorLevel;
    gFlightCell = gCursorCell;
    gCellSplit = 0;
    gFlightClipSet = gLoadedOpenClipSet;
    if (*(const short *)((gFlightLevel * sizeof(*cells) + gFlightCell * sizeof(**cells)) + (u_long)cells) != -1) {
        DrawSync(0);
        selectedModel = gGridCells[gFlightLevel][gFlightCell].modelIndex;
        GsGetLws(&gCellModels[selectedModel + 1].coord, &localWorld, &localScreen);
        gCellModels[selectedModel + 1].object.attribute |= 0x80000000;
        InitModelCoordinate(&gCellModels[99]);
        SetModelTmd(gEmptyCellTmd, &gCellModels[99]);
        LinkModelObjects(&gCellModels[99], &gModels[5].coord);

        cell = gFlightCell;
        gCellModels[99].coord.coord.t[0] = gCellPositions[cell].vx;
        gCellModels[99].coord.coord.t[1] = gCellPositions[cell].vy + 100 - gFlightLevel * 900;
        gCellModels[99].coord.coord.t[2] = gCellPositions[cell].vz;
        gCellModels[99].rot.vy += (cell * 45 + 22.5 - (unsigned int)gFlightCell * 360) * 4096 / 360;
        gCellModels[99].object.attribute = 0x50000000;
        UpdateModelMatrix(&gCellModels[99]);

        GsGetTimInfo(gCursorHighlightTimFlags, &image);
        pixels.x = image.px;
        pixels.y = image.py;
        pixels.w = image.pw;
        pixels.h = image.ph;
        clut.x = image.cx;
        clut.y = image.cy;
        clut.w = image.cw;
        clut.h = image.ch;
        GsGetTimInfo(gFlyingCellTimFlags, &image);
        MoveImage(&pixels, image.px, image.py);
        DrawSync(0);
        MoveImage(&clut, image.cx, image.cy);
        DrawSync(0);

        PushMatrix();
        angle.vx = 0;
        angle.vz = 0;
        angle.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
        RotMatrix(&angle, &rotation);
        SetRotMatrix(&rotation);
        input.vx = localWorld.t[0];
        input.vy = localWorld.t[1];
        input.vz = localWorld.t[2];
        ApplyRotMatrix(&input, &point);
        gBezierPoints[0][0] = point.vx;
        gBezierPoints[0][1] = (double)point.vy - gGridLevel * 900;
        gBezierPoints[0][2] = point.vz;
        input.vx = gFlightPathControls[gFlightClipSet].vx;
        input.vy = gFlightPathControls[gFlightClipSet].vy;
        input.vz = gFlightPathControls[gFlightClipSet].vz;
        ApplyRotMatrix(&input, &point);
        gBezierPoints[1][0] = point.vx;
        gBezierPoints[1][1] = (double)point.vy - gGridLevel * 900;
        gBezierPoints[1][2] = point.vz;
        input.vx = gFlightPathEnds[gFlightClipSet].vx;
        input.vy = gFlightPathEnds[gFlightClipSet].vy;
        input.vz = gFlightPathEnds[gFlightClipSet].vz;
        ApplyRotMatrix(&input, &point);
        gBezierPoints[2][0] = point.vx;
        gBezierPoints[2][1] = (double)point.vy - gGridLevel * 900;
        gBezierPoints[2][2] = point.vz;
        PopMatrix();
        WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);

        gSavedPointDeltas[0] = gPointDeltas[0];
        gSavedPointDeltaStepsLeft = gPointDeltaStepsLeft;
        gSavedPointDeltaMode = gPointDeltaMode;
        info = *gCellModels[99].objects;
        for (i = 0; i < info.vern; i++) {
            gFlyingCellPoints[i] = gFlatCellPoints[i];
            SetTmdVertex(i, &gFlatCellPoints[i], &info);
        }
        gPointDeltaMode = POINT_DELTA_CELL_FLIGHT;
        InitPointDeltas(gCubeCellPoints, gFlyingCellPoints, (short)((int)gFlightStepCounts[gFlightClipSet] - 1), 8);
        gFlightStep = 0.0;
        gGridBusy = 255;
        gCellFlying = 255;
        gFlightEnding = 0;
    } else {
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
    }
}

void StepCellFlight(void)
{
    double pos[3];
    SVECTOR unused; /* 8 bytes; nothing uses it */
    struct TMD_STRUCT info;
    u_int i;

    DrawSync(0);
    if (gFlightStep <= gFlightStepCounts[gFlightClipSet]) {
        if (gPointDeltaStepsLeft > 0 && gPointDeltaMode == POINT_DELTA_CELL_FLIGHT) {
            ApplyPointDeltas(gFlyingCellPoints);
            info = *gCellModels[99].objects;
            for (i = 0; i < info.vern; i++) {
                SetTmdVertex(i, &gFlyingCellPoints[i], &info);
            }
        }
        EvalBezier(3, gWeightedBezierPoints, pos, gFlightStep / gFlightStepCounts[gFlightClipSet]);
        gCellModels[99].coord.coord.t[0] = pos[0];
        gCellModels[99].coord.coord.t[1] = pos[1];
        gCellModels[99].coord.coord.t[2] = pos[2];
        gCellModels[99].rot.vy = gGridHeading * 512;
        UpdateModelMatrix(&gCellModels[99]);
        gFlightStep += 1.0;
    } else {
        gPointDeltas[0] = gSavedPointDeltas[0];
        gPointDeltaStepsLeft = gSavedPointDeltaStepsLeft;
        gPointDeltaMode = gSavedPointDeltaMode;
        switch (gFlightClipSet) {
        case OPEN_CLIP_SPIN_AWAY:
            gFlightEnding = 255;
            gFlightEndFramesLeft = 21;
            gFlightEndStep = 10;
            PlaySound(12);
            break;
        case OPEN_CLIP_COLLAPSE_SPLIT:
            gFlightEnding = 255;
            gFlightEndFramesLeft = 31;
            gFlightEndStep = 15;
            PlaySound(12);
            gSavedPointDeltas[0] = gPointDeltas[0];
            gSavedPointDeltaStepsLeft = gPointDeltaStepsLeft;
            gSavedPointDeltaMode = gPointDeltaMode;
            gPointDeltaMode = POINT_DELTA_CELL_FLIGHT;
            InitPointDeltas(gCollapsedCellPoints, gFlyingCellPoints, 4, 8);
            break;
        case OPEN_CLIP_REFUSE_SPARK:
            gFlightEnding = 255;
            gFlightEndFramesLeft = 25;
            break;
        case OPEN_CLIP_REFUSE_RETURN:
            gFlightEnding = 255;
            gFlightEndFramesLeft = 22;
            gFlightEndStep = 0;
            break;
        case OPEN_CLIP_REFUSE_VANISH:
            gFlightEnding = 255;
            gFlightEndFramesLeft = 63;
            break;
        default:
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
            gCellFlying = 0;
            break;
        }
        info = *gCellModels[99].objects;
        for (i = 0; i < info.norn; i++) {
            SetTmdNormal(i, &gCellFaceNormals[i], &info);
        }
    }
}

void StepCellFlightEnd(void)
{
    GsIMAGE image;
    RECT rect;
    MATRIX lw;
    MATRIX ls;
    MATRIX m;
    SVECTOR v;
    SVECTOR ang;
    SVECTOR pts[20];
    VECTOR out;
    VECTOR unused; /* 16 bytes; nothing uses it */
    double pos[3];
    GsGLINE line;
    struct TMD_STRUCT info;
    int i;

    DrawSync(0);
    switch (gFlightClipSet) {
    case OPEN_CLIP_SPIN_AWAY:
        PushMatrix();
        ang.vx = 0;
        ang.vz = 0;
        ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
        RotMatrix(&ang, &m);
        SetRotMatrix(&m);
        if (gFlightEndStep != 0) {
            v.vx = gClipSet0FlightEndPath[10 - gFlightEndStep].vx;
            v.vy = gClipSet0FlightEndPath[10 - gFlightEndStep].vy;
            v.vz = gClipSet0FlightEndPath[10 - gFlightEndStep].vz;
            ApplyRotMatrix(&v, &out);
            gCellModels[99].coord.coord.t[0] = out.vx;
            gCellModels[99].coord.coord.t[1] = out.vy - gGridLevel * 900;
            gCellModels[99].coord.coord.t[2] = out.vz;
            PopMatrix();
            gFlightEndStep--;
        } else {
            v.vx = 20;
            v.vy = 0;
            v.vz = 150;
            ApplyRotMatrix(&v, &out);
            gCellModels[99].coord.coord.t[0] -= out.vx;
            gCellModels[99].coord.coord.t[2] -= out.vz;
            PopMatrix();
            gCellModels[99].rot.vy += 45;
            gCellModels[99].rot.vz += 45;
        }
        UpdateModelMatrix(&gCellModels[99]);
        if (--gFlightEndFramesLeft == 11) {
            PlaySound(13);
        }
        if (gFlightEndFramesLeft == 9) {
            PlaySound(14);
        }
        if (gFlightEndFramesLeft == 0) {
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
        }
        break;
    case OPEN_CLIP_COLLAPSE_SPLIT:
        if (gFlightEndStep != 0) {
            PushMatrix();
            ang.vx = 0;
            ang.vz = 0;
            ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
            RotMatrix(&ang, &m);
            SetRotMatrix(&m);
            v.vx = gClipSet1FlightEndPath[15 - gFlightEndStep].vx;
            v.vy = gClipSet1FlightEndPath[15 - gFlightEndStep].vy;
            v.vz = gClipSet1FlightEndPath[15 - gFlightEndStep].vz;
            ApplyRotMatrix(&v, &out);
            gCellModels[99].coord.coord.t[0] = out.vx;
            gCellModels[99].coord.coord.t[1] = out.vy - gGridLevel * 900;
            gCellModels[99].coord.coord.t[2] = out.vz;
            PopMatrix();
            gCellModels[99].rot.vy += 45;
            gCellModels[99].rot.vz += 45;
            gFlightEndStep--;
            UpdateModelMatrix(&gCellModels[99]);
        }
        if (gFlightEndStep < 5 && gPointDeltaStepsLeft > 0 && gPointDeltaMode == POINT_DELTA_CELL_FLIGHT) {
            ApplyPointDeltas(gFlyingCellPoints);
            info = *gCellModels[99].objects;
            for (i = 0; i < info.vern; i++) {
                SetTmdVertex(i, &gFlyingCellPoints[i], &info);
            }
        } else if (gPointDeltaStepsLeft == 0 && gPointDeltaMode == POINT_DELTA_CELL_FLIGHT) {
            gPointDeltaStepsLeft = gSavedPointDeltaStepsLeft;
            gPointDeltaMode = gSavedPointDeltaMode;
            gPointDeltas[0] = gSavedPointDeltas[0];
        }
        if (gFlightEndFramesLeft == 13) {
            PlaySound(15);
            GsGetLws(&gCellModels[99].coord, &lw, &ls);
            gCellSplitX = lw.t[0];
            gCellSplitY = lw.t[1];
            gCellSplitZ = lw.t[2];
            ang.vx = 0;
            ang.vz = 0;
            ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
            PushMatrix();
            RotMatrix(&ang, &m);
            SetRotMatrix(&m);
            for (i = 0; i < 4; i++) {
                InitModelCoordinate(&gCellModels[99 + i]);
                SetModelTmd(gCellPieceTmds[i], &gCellModels[99 + i]);
                LinkModelObjects(&gCellModels[99 + i], &gModels[5].coord);
                v.vx = gClipSet1FlightEndPath[14].vx;
                v.vy = gClipSet1FlightEndPath[14].vy;
                v.vz = gClipSet1FlightEndPath[14].vz;
                ApplyRotMatrix(&v, &out);
                gCellModels[99 + i].coord.coord.t[0] = out.vx;
                gCellModels[99 + i].coord.coord.t[1] = out.vy - gGridLevel * 900;
                gCellModels[99 + i].coord.coord.t[2] = out.vz;
                UpdateModelMatrix(&gCellModels[99 + i]);
                gCellSplit = 255;
            }
            PopMatrix();
        } else if (gFlightEndFramesLeft < 13) {
            if (gFlightEndFramesLeft == 11) {
                PlaySound(16);
            }
            ang.vx = 0;
            ang.vz = 0;
            ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
            PushMatrix();
            RotMatrix(&ang, &m);
            SetRotMatrix(&m);
            for (i = 0; i < 4; i++) {
                v.vx = gCellPieceSteps[i].vx;
                v.vy = gCellPieceSteps[i].vy;
                v.vz = gCellPieceSteps[i].vz;
                ApplyRotMatrix(&v, &out);
                gCellModels[99 + i].coord.coord.t[0] -= out.vx;
                gCellModels[99 + i].coord.coord.t[2] -= out.vz;
                gCellModels[99 + i].rot.vy += 45;
                gCellModels[99 + i].rot.vz += 45;
                UpdateModelMatrix(&gCellModels[99 + i]);
            }
            PopMatrix();
        }
        if (gFlightEndFramesLeft == 11 || gFlightEndFramesLeft == 12) {
            for (i = 0; i < 20; i++) {
                InitChildCoordinatePair(&gFlightLines[i], &gModels[5].coord);
                PushMatrix();
                ang.vx = 0;
                ang.vz = 0;
                ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
                RotMatrix(&ang, &m);
                SetRotMatrix(&m);
                v.vx = gCellSplitX;
                v.vy = gCellSplitY - gGridLevel * 900;
                v.vz = gCellSplitZ;
                ApplyRotMatrix(&v, &out);
                gFlightLines[i].coord[0].coord.t[0] = out.vx;
                gFlightLines[i].coord[0].coord.t[1] = out.vy;
                gFlightLines[i].coord[0].coord.t[2] = out.vz;
                v.vx = (gCellSplitX - gSineTable[rand() & 0x7ff]) >> 8;
                v.vy = ((gCellSplitY - gSineTable[rand() & 0x7ff]) >> 8) - gGridLevel * 900;
                v.vz = (gCellSplitZ - gSineTable[rand() & 0x7ff]) >> 8;
                ApplyRotMatrix(&v, &out);
                gFlightLines[i].coord[1].coord.t[0] = out.vx;
                gFlightLines[i].coord[1].coord.t[1] = out.vy;
                gFlightLines[i].coord[1].coord.t[2] = out.vz;
                PopMatrix();
                line.attribute = 0x50000000;
                line.r0 = 255;
                line.g0 = rand();
                line.b0 = 0;
                line.b1 = 0;
                line.g1 = 0;
                line.r1 = 0;
                UpdateCoordinatePairMatrices(&gFlightLines[i]);
                ProjectCoordinatePairToScreen(gFlightLines[i].coord, &line.x0, &line.x1);
                if (IsPointOffScreen(&line.x0, 100) == 0) {
                    GsSortGLine(&line, &gOt[gActiveBuffer], 350);
                }
            }
        }
        if (--gFlightEndFramesLeft == 0) {
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
            gCellSplit = 0;
        }
        break;
    case OPEN_CLIP_REFUSE_SPARK:
        if (gFlightEndFramesLeft != 0) {
            gFlightEndFramesLeft--;
        }
        if (gFlightEndFramesLeft < 6) {
            gCellModels[99].object.attribute |= 0x80000000;
        } else {
            if (gFlightEndFramesLeft == 12) {
                i = 0;
                UploadTim(&gFlyingCellTim);
                GsGetTimInfo(gCellTims[gGridCells[gFlightLevel][gFlightCell].modelType] + 1, &image);
                rect.x = image.cx;
                rect.y = image.cy;
                rect.w = image.cw;
                rect.h = image.ch;
                GsGetTimInfo(gFlyingCellTimFlags, &image);
                MoveImage(&rect, image.px, image.py);
                DrawSync(0);
                info = *gCellModels[99].objects;
                for (; i < info.norn; i++) {
                    SetTmdNormal(i, &gCellFaceNormals[i], &info);
                }
                PlaySound(17);
            }
            if (gFlightEndFramesLeft < 12) {
                GsGetLws(&gCellModels[99].coord, &lw, &ls);
                for (i = 0; i < 5; i++) {
                    InitChildCoordinatePair(&gFlightLines[i], &gModels[5].coord);
                    PushMatrix();
                    ang.vx = 0;
                    ang.vz = 0;
                    ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
                    RotMatrix(&ang, &m);
                    SetRotMatrix(&m);
                    v.vx = lw.t[0];
                    v.vy = lw.t[1] - gGridLevel * 900;
                    v.vz = lw.t[2];
                    ApplyRotMatrix(&v, &out);
                    gFlightLines[i].coord[0].coord.t[0] = out.vx;
                    gFlightLines[i].coord[0].coord.t[1] = out.vy;
                    gFlightLines[i].coord[0].coord.t[2] = out.vz;
                    v.vx = (lw.t[0] - gSineTable[rand() & 0x7ff]) >> 7;
                    v.vy = ((lw.t[1] - gSineTable[rand() & 0x7ff]) >> 7) - gGridLevel * 900;
                    v.vz = (lw.t[2] - gSineTable[rand() & 0x7ff]) >> 7;
                    ApplyRotMatrix(&v, &out);
                    gFlightLines[i].coord[1].coord.t[0] = out.vx;
                    gFlightLines[i].coord[1].coord.t[1] = out.vy;
                    gFlightLines[i].coord[1].coord.t[2] = out.vz;
                    PopMatrix();
                    line.attribute = 0x50000000;
                    line.r0 = 255;
                    line.g0 = rand();
                    line.b0 = 0;
                    line.b1 = 0;
                    line.g1 = 0;
                    line.r1 = 0;
                    UpdateCoordinatePairMatrices(&gFlightLines[i]);
                    ProjectCoordinatePairToScreen(gFlightLines[i].coord, &line.x0, &line.x1);
                    if (IsPointOffScreen(&line.x0, 100) == 0) {
                        GsSortGLine(&line, &gOt[gActiveBuffer], 350);
                    }
                }
            }
            gCellModels[99].rot.vy += 91;
            gCellModels[99].rot.vz += 91;
            UpdateModelMatrix(&gCellModels[99]);
        }
        if (gFlightEndFramesLeft == 0) {
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
            gCellFlying = 0;
            PlaySound(31);
        }
        break;
    case OPEN_CLIP_REFUSE_RETURN:
        if (--gFlightEndFramesLeft == 6) {
            GsGetLws(&gCellModels[99].coord, &lw, &ls);
            PushMatrix();
            ang.vx = 0;
            ang.vz = 0;
            ang.vy = (gGridHeading * 45 - gGridHeading * 360) * 4096 / 360;
            RotMatrix(&ang, &m);
            SetRotMatrix(&m);
            v.vx = lw.t[0];
            v.vy = lw.t[1];
            v.vz = lw.t[2];
            ApplyRotMatrix(&v, &out);
            gBezierPoints[0][0] = out.vx;
            gBezierPoints[0][1] = (double)out.vy - gGridLevel * 900;
            gBezierPoints[0][2] = out.vz;
            v.vx = gFlightPathControls[gFlightClipSet].vx;
            v.vy = gFlightPathControls[gFlightClipSet].vy;
            v.vz = gFlightPathControls[gFlightClipSet].vz;
            ApplyRotMatrix(&v, &out);
            gBezierPoints[1][0] = out.vx;
            gBezierPoints[1][1] = (double)out.vy - gGridLevel * 900;
            gBezierPoints[1][2] = out.vz;
            v.vx = gCellPositions[gFlightCell].vx;
            v.vy = gCellPositions[gFlightCell].vy + 100;
            v.vz = gCellPositions[gFlightCell].vz;
            ApplyRotMatrix(&v, &out);
            gBezierPoints[2][0] = out.vx;
            gBezierPoints[2][1] = (double)out.vy - gGridLevel * 900;
            gBezierPoints[2][2] = out.vz;
            PopMatrix();
            WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);
        }
        if (gFlightEndFramesLeft < 6) {
            EvalBezier(3, gWeightedBezierPoints, pos, gFlightEndStep / 6.0);
            gCellModels[99].coord.coord.t[0] = pos[0];
            gCellModels[99].coord.coord.t[1] = pos[1];
            gCellModels[99].coord.coord.t[2] = pos[2];
            gCellModels[99].rot.vy = gGridHeading * 512;
            UpdateModelMatrix(&gCellModels[99]);
            gFlightEndStep++;
        }
        if (gFlightEndFramesLeft == 12 || gFlightEndFramesLeft == 8) {
            PlaySound(18);
        }
        if (gFlightEndFramesLeft == 0) {
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
            gCellFlying = 0;
            gFlightEndStep = 0;
        }
        break;
    case OPEN_CLIP_REFUSE_VANISH:
        if (--gFlightEndFramesLeft == 37) {
            for (i = 0; i < 20; i++) {
                pts[i].vx = (gSineTable[i] >> 16) * 10;
                pts[i].vy = lw.t[1];
                pts[i].vz = (gSineTable[i] >> 16) * 10;
                gFlightEndCoords[i].coord.coord.t[0] = lw.t[0];
                gFlightEndCoords[i].coord.coord.t[1] = lw.t[1];
                gFlightEndCoords[i].coord.coord.t[2] = lw.t[2];
            }
        }
        if (gFlightEndFramesLeft < 37) {
            gCellModels[99].object.attribute |= 0x80000000;
        }
        if (gFlightEndFramesLeft == 53 || gFlightEndFramesLeft == 49 || gFlightEndFramesLeft == 45 || gFlightEndFramesLeft == 41) {
            PlaySound(18);
        }
        if (gFlightEndFramesLeft == 31) {
            PlaySound(19);
        }
        if (gFlightEndFramesLeft == 0) {
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gFlightEnding = 0;
            gCellFlying = 0;
            PlaySound(31);
        }
        break;
    default:
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
        gFlightEnding = 0;
        gCellFlying = 0;
        break;
    }
}

void InitClipFrameSprites(void)
{
    RECT rect;
    GsSPRITE *sprites;
    u_short tpage;

    tpage = GetTPage(2, 1, 320, 0);
    sprites = gClipFrameSprites;
    sprites[0].tpage = tpage;
    sprites[1].tpage = GetTPage(2, 1, 576, 0);
    sprites[2].tpage = GetTPage(2, 1, 320, 256);
    sprites[3].tpage = GetTPage(2, 1, 576, 256);
    rect.x = 320;
    rect.w = 320;
    rect.y = 0;
    rect.h = 480;
    ClearImage2(&rect, 0, 0, 0);
}

void ResetGridSceneToCell(unsigned int level, unsigned int cell)
{
    unsigned char unusedFrame[8]; /* 8 bytes; nothing uses it */

    InitView();
    SetViewLights();
    InitGridScene();
    if (level >= (unsigned int)GetGridLevelCount()) {
        level = GetGridLevelCount() - 1;
    }
    if (cell >= 24) {
        cell = 23;
    }
    gSceneCommand = SCENE_CMD_TRAVEL_TO_CURSOR;
    gCursorLevel = level;
    gCursorCell = cell;
    StartViewDescent();
    SetViewLights();
}

void StepOrFinishTravelToCursor(void)
{
    if (gViewMode == VIEW_DESCENT) {
        while (gGridBusy != 0) {
            StepTravelToCursor();
        }
        gGridBusy = 0xff;
    } else {
        StepTravelToCursor();
    }
}

void StartViewDescent(void)
{
    SetViewMode(VIEW_DESCENT);
    StartViewPath(VIEW_DESCENT);
}

void StepViewDescent(void)
{
    if (gViewMode == VIEW_DESCENT && gSceneCommand != SCENE_CMD_TRAVEL_TO_CURSOR) {
        StepViewPath();
        if (gViewPathStep < 50.0) {
            gClipFrameOffsetX = 12;
        } else {
            gClipFrameOffsetX = 0;
        }
        if (gDescentPathDone != 0) {
            gDescentPathDone = 0;
            gClipFrameOffsetX = 0;
            gGridBusy = 0;
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gViewMode = VIEW_FRONT;
            SetViewLights();
            InitRing();
        }
    }
}

/* Entry i is sin(i/2048 of a turn) * 65536; the extra 512 entries let a cosine read a quarter turn on. */
int gSineTable[2560] = {
    0, 201, 402, 603, 804, 1005, 1206, 1407, 1608, 1809, 2010, 2211,
    2412, 2613, 2814, 3015, 3216, 3417, 3617, 3818, 4019, 4219, 4420, 4621,
    4821, 5022, 5222, 5422, 5623, 5823, 6023, 6224, 6424, 6624, 6824, 7024,
    7224, 7423, 7623, 7823, 8022, 8222, 8421, 8621, 8820, 9019, 9218, 9417,
    9616, 9815, 10014, 10212, 10411, 10609, 10808, 11006, 11204, 11402, 11600, 11798,
    11996, 12193, 12391, 12588, 12785, 12983, 13180, 13376, 13573, 13770, 13966, 14163,
    14359, 14555, 14751, 14947, 15143, 15338, 15534, 15729, 15924, 16119, 16314, 16508,
    16703, 16897, 17091, 17285, 17479, 17673, 17867, 18060, 18253, 18446, 18639, 18832,
    19024, 19216, 19409, 19600, 19792, 19984, 20175, 20366, 20557, 20748, 20939, 21129,
    21320, 21510, 21699, 21889, 22078, 22268, 22457, 22645, 22834, 23022, 23210, 23398,
    23586, 23774, 23961, 24148, 24335, 24521, 24708, 24894, 25080, 25265, 25451, 25636,
    25821, 26005, 26190, 26374, 26558, 26742, 26925, 27108, 27291, 27474, 27656, 27838,
    28020, 28202, 28383, 28564, 28745, 28926, 29106, 29286, 29466, 29645, 29824, 30003,
    30182, 30360, 30538, 30716, 30893, 31071, 31248, 31424, 31600, 31776, 31952, 32127,
    32303, 32477, 32652, 32826, 33000, 33173, 33347, 33520, 33692, 33865, 34037, 34208,
    34380, 34551, 34721, 34892, 35062, 35231, 35401, 35570, 35738, 35907, 36075, 36243,
    36410, 36577, 36744, 36910, 37076, 37241, 37407, 37572, 37736, 37900, 38064, 38228,
    38391, 38554, 38716, 38878, 39040, 39201, 39362, 39523, 39683, 39843, 40002, 40161,
    40320, 40478, 40636, 40794, 40951, 41108, 41264, 41420, 41576, 41731, 41886, 42040,
    42194, 42348, 42501, 42654, 42806, 42958, 43110, 43261, 43412, 43562, 43713, 43862,
    44011, 44160, 44308, 44456, 44604, 44751, 44898, 45044, 45190, 45335, 45480, 45625,
    45769, 45912, 46056, 46199, 46341, 46483, 46624, 46765, 46906, 47046, 47186, 47325,
    47464, 47603, 47741, 47878, 48015, 48152, 48288, 48424, 48559, 48694, 48828, 48962,
    49095, 49228, 49361, 49493, 49624, 49756, 49886, 50016, 50146, 50275, 50404, 50532,
    50660, 50787, 50914, 51041, 51166, 51292, 51417, 51541, 51665, 51789, 51911, 52034,
    52156, 52277, 52398, 52519, 52639, 52759, 52878, 52996, 53114, 53232, 53349, 53465,
    53581, 53697, 53812, 53926, 54040, 54154, 54267, 54379, 54491, 54603, 54714, 54824,
    54934, 55043, 55152, 55260, 55368, 55476, 55582, 55689, 55794, 55900, 56004, 56108,
    56212, 56315, 56418, 56520, 56621, 56722, 56823, 56923, 57022, 57121, 57219, 57317,
    57414, 57511, 57607, 57703, 57798, 57892, 57986, 58079, 58172, 58265, 58356, 58448,
    58538, 58628, 58718, 58807, 58896, 58983, 59071, 59158, 59244, 59330, 59415, 59499,
    59583, 59667, 59750, 59832, 59914, 59995, 60075, 60156, 60235, 60314, 60392, 60470,
    60547, 60624, 60700, 60776, 60851, 60925, 60999, 61072, 61145, 61217, 61288, 61359,
    61429, 61499, 61568, 61637, 61705, 61772, 61839, 61906, 61971, 62036, 62101, 62165,
    62228, 62291, 62353, 62415, 62476, 62536, 62596, 62655, 62714, 62772, 62830, 62886,
    62943, 62998, 63054, 63108, 63162, 63215, 63268, 63320, 63372, 63423, 63473, 63523,
    63572, 63621, 63668, 63716, 63763, 63809, 63854, 63899, 63944, 63987, 64031, 64073,
    64115, 64156, 64197, 64237, 64277, 64316, 64354, 64392, 64429, 64465, 64501, 64536,
    64571, 64605, 64639, 64672, 64704, 64735, 64766, 64797, 64827, 64856, 64884, 64912,
    64940, 64967, 64993, 65018, 65043, 65067, 65091, 65114, 65137, 65159, 65180, 65200,
    65220, 65240, 65259, 65277, 65294, 65311, 65328, 65343, 65358, 65373, 65387, 65400,
    65413, 65425, 65436, 65447, 65457, 65467, 65476, 65484, 65492, 65499, 65505, 65511,
    65516, 65521, 65525, 65528, 65531, 65533, 65535, 65536, 65536, 65536, 65535, 65533,
    65531, 65528, 65525, 65521, 65516, 65511, 65505, 65499, 65492, 65484, 65476, 65467,
    65457, 65447, 65436, 65425, 65413, 65400, 65387, 65373, 65358, 65343, 65328, 65311,
    65294, 65277, 65259, 65240, 65220, 65200, 65180, 65159, 65137, 65114, 65091, 65067,
    65043, 65018, 64993, 64967, 64940, 64912, 64884, 64856, 64827, 64797, 64766, 64735,
    64704, 64672, 64639, 64605, 64571, 64536, 64501, 64465, 64429, 64392, 64354, 64316,
    64277, 64237, 64197, 64156, 64115, 64073, 64031, 63987, 63944, 63899, 63854, 63809,
    63763, 63716, 63668, 63621, 63572, 63523, 63473, 63423, 63372, 63320, 63268, 63215,
    63162, 63108, 63054, 62998, 62943, 62886, 62830, 62772, 62714, 62655, 62596, 62536,
    62476, 62415, 62353, 62291, 62228, 62165, 62101, 62036, 61971, 61906, 61839, 61772,
    61705, 61637, 61568, 61499, 61429, 61359, 61288, 61217, 61145, 61072, 60999, 60925,
    60851, 60776, 60700, 60624, 60547, 60470, 60392, 60314, 60235, 60156, 60075, 59995,
    59914, 59832, 59750, 59667, 59583, 59499, 59415, 59330, 59244, 59158, 59071, 58983,
    58896, 58807, 58718, 58628, 58538, 58448, 58356, 58265, 58172, 58079, 57986, 57892,
    57798, 57703, 57607, 57511, 57414, 57317, 57219, 57121, 57022, 56923, 56823, 56722,
    56621, 56520, 56418, 56315, 56212, 56108, 56004, 55900, 55794, 55689, 55582, 55476,
    55368, 55260, 55152, 55043, 54934, 54824, 54714, 54603, 54491, 54379, 54267, 54154,
    54040, 53926, 53812, 53697, 53581, 53465, 53349, 53232, 53114, 52996, 52878, 52759,
    52639, 52519, 52398, 52277, 52156, 52034, 51911, 51789, 51665, 51541, 51417, 51292,
    51166, 51041, 50914, 50787, 50660, 50532, 50404, 50275, 50146, 50016, 49886, 49756,
    49624, 49493, 49361, 49228, 49095, 48962, 48828, 48694, 48559, 48424, 48288, 48152,
    48015, 47878, 47741, 47603, 47464, 47325, 47186, 47046, 46906, 46765, 46624, 46483,
    46341, 46199, 46056, 45912, 45769, 45625, 45480, 45335, 45190, 45044, 44898, 44751,
    44604, 44456, 44308, 44160, 44011, 43862, 43713, 43562, 43412, 43261, 43110, 42958,
    42806, 42654, 42501, 42348, 42194, 42040, 41886, 41731, 41576, 41420, 41264, 41108,
    40951, 40794, 40636, 40478, 40320, 40161, 40002, 39843, 39683, 39523, 39362, 39201,
    39040, 38878, 38716, 38554, 38391, 38228, 38064, 37900, 37736, 37572, 37407, 37241,
    37076, 36910, 36744, 36577, 36410, 36243, 36075, 35907, 35738, 35570, 35401, 35231,
    35062, 34892, 34721, 34551, 34380, 34208, 34037, 33865, 33692, 33520, 33347, 33173,
    33000, 32826, 32652, 32477, 32303, 32127, 31952, 31776, 31600, 31424, 31248, 31071,
    30893, 30716, 30538, 30360, 30182, 30003, 29824, 29645, 29466, 29286, 29106, 28926,
    28745, 28564, 28383, 28202, 28020, 27838, 27656, 27474, 27291, 27108, 26925, 26742,
    26558, 26374, 26190, 26005, 25821, 25636, 25451, 25265, 25080, 24894, 24708, 24521,
    24335, 24148, 23961, 23774, 23586, 23398, 23210, 23022, 22834, 22645, 22457, 22268,
    22078, 21889, 21699, 21510, 21320, 21129, 20939, 20748, 20557, 20366, 20175, 19984,
    19792, 19600, 19409, 19216, 19024, 18832, 18639, 18446, 18253, 18060, 17867, 17673,
    17479, 17285, 17091, 16897, 16703, 16508, 16314, 16119, 15924, 15729, 15534, 15338,
    15143, 14947, 14751, 14555, 14359, 14163, 13966, 13770, 13573, 13376, 13180, 12983,
    12785, 12588, 12391, 12193, 11996, 11798, 11600, 11402, 11204, 11006, 10808, 10609,
    10411, 10212, 10014, 9815, 9616, 9417, 9218, 9019, 8820, 8621, 8421, 8222,
    8022, 7823, 7623, 7423, 7224, 7024, 6824, 6624, 6424, 6224, 6023, 5823,
    5623, 5422, 5222, 5022, 4821, 4621, 4420, 4219, 4019, 3818, 3617, 3417,
    3216, 3015, 2814, 2613, 2412, 2211, 2010, 1809, 1608, 1407, 1206, 1005,
    804, 603, 402, 201, 0, -201, -402, -603, -804, -1005, -1206, -1407,
    -1608, -1809, -2010, -2211, -2412, -2613, -2814, -3015, -3216, -3417, -3617, -3818,
    -4019, -4219, -4420, -4621, -4821, -5022, -5222, -5422, -5623, -5823, -6023, -6224,
    -6424, -6624, -6824, -7024, -7224, -7423, -7623, -7823, -8022, -8222, -8421, -8621,
    -8820, -9019, -9218, -9417, -9616, -9815, -10014, -10212, -10411, -10609, -10808, -11006,
    -11204, -11402, -11600, -11798, -11996, -12193, -12391, -12588, -12785, -12983, -13180, -13376,
    -13573, -13770, -13966, -14163, -14359, -14555, -14751, -14947, -15143, -15338, -15534, -15729,
    -15924, -16119, -16314, -16508, -16703, -16897, -17091, -17285, -17479, -17673, -17867, -18060,
    -18253, -18446, -18639, -18832, -19024, -19216, -19409, -19600, -19792, -19984, -20175, -20366,
    -20557, -20748, -20939, -21129, -21320, -21510, -21699, -21889, -22078, -22268, -22457, -22645,
    -22834, -23022, -23210, -23398, -23586, -23774, -23961, -24148, -24335, -24521, -24708, -24894,
    -25080, -25265, -25451, -25636, -25821, -26005, -26190, -26374, -26558, -26742, -26925, -27108,
    -27291, -27474, -27656, -27838, -28020, -28202, -28383, -28564, -28745, -28926, -29106, -29286,
    -29466, -29645, -29824, -30003, -30182, -30360, -30538, -30716, -30893, -31071, -31248, -31424,
    -31600, -31776, -31952, -32127, -32303, -32477, -32652, -32826, -33000, -33173, -33347, -33520,
    -33692, -33865, -34037, -34208, -34380, -34551, -34721, -34892, -35062, -35231, -35401, -35570,
    -35738, -35907, -36075, -36243, -36410, -36577, -36744, -36910, -37076, -37241, -37407, -37572,
    -37736, -37900, -38064, -38228, -38391, -38554, -38716, -38878, -39040, -39201, -39362, -39523,
    -39683, -39843, -40002, -40161, -40320, -40478, -40636, -40794, -40951, -41108, -41264, -41420,
    -41576, -41731, -41886, -42040, -42194, -42348, -42501, -42654, -42806, -42958, -43110, -43261,
    -43412, -43562, -43713, -43862, -44011, -44160, -44308, -44456, -44604, -44751, -44898, -45044,
    -45190, -45335, -45480, -45625, -45769, -45912, -46056, -46199, -46341, -46483, -46624, -46765,
    -46906, -47046, -47186, -47325, -47464, -47603, -47741, -47878, -48015, -48152, -48288, -48424,
    -48559, -48694, -48828, -48962, -49095, -49228, -49361, -49493, -49624, -49756, -49886, -50016,
    -50146, -50275, -50404, -50532, -50660, -50787, -50914, -51041, -51166, -51292, -51417, -51541,
    -51665, -51789, -51911, -52034, -52156, -52277, -52398, -52519, -52639, -52759, -52878, -52996,
    -53114, -53232, -53349, -53465, -53581, -53697, -53812, -53926, -54040, -54154, -54267, -54379,
    -54491, -54603, -54714, -54824, -54934, -55043, -55152, -55260, -55368, -55476, -55582, -55689,
    -55794, -55900, -56004, -56108, -56212, -56315, -56418, -56520, -56621, -56722, -56823, -56923,
    -57022, -57121, -57219, -57317, -57414, -57511, -57607, -57703, -57798, -57892, -57986, -58079,
    -58172, -58265, -58356, -58448, -58538, -58628, -58718, -58807, -58896, -58983, -59071, -59158,
    -59244, -59330, -59415, -59499, -59583, -59667, -59750, -59832, -59914, -59995, -60075, -60156,
    -60235, -60314, -60392, -60470, -60547, -60624, -60700, -60776, -60851, -60925, -60999, -61072,
    -61145, -61217, -61288, -61359, -61429, -61499, -61568, -61637, -61705, -61772, -61839, -61906,
    -61971, -62036, -62101, -62165, -62228, -62291, -62353, -62415, -62476, -62536, -62596, -62655,
    -62714, -62772, -62830, -62886, -62943, -62998, -63054, -63108, -63162, -63215, -63268, -63320,
    -63372, -63423, -63473, -63523, -63572, -63621, -63668, -63716, -63763, -63809, -63854, -63899,
    -63944, -63987, -64031, -64073, -64115, -64156, -64197, -64237, -64277, -64316, -64354, -64392,
    -64429, -64465, -64501, -64536, -64571, -64605, -64639, -64672, -64704, -64735, -64766, -64797,
    -64827, -64856, -64884, -64912, -64940, -64967, -64993, -65018, -65043, -65067, -65091, -65114,
    -65137, -65159, -65180, -65200, -65220, -65240, -65259, -65277, -65294, -65311, -65328, -65343,
    -65358, -65373, -65387, -65400, -65413, -65425, -65436, -65447, -65457, -65467, -65476, -65484,
    -65492, -65499, -65505, -65511, -65516, -65521, -65525, -65528, -65531, -65533, -65535, -65536,
    -65536, -65536, -65535, -65533, -65531, -65528, -65525, -65521, -65516, -65511, -65505, -65499,
    -65492, -65484, -65476, -65467, -65457, -65447, -65436, -65425, -65413, -65400, -65387, -65373,
    -65358, -65343, -65328, -65311, -65294, -65277, -65259, -65240, -65220, -65200, -65180, -65159,
    -65137, -65114, -65091, -65067, -65043, -65018, -64993, -64967, -64940, -64912, -64884, -64856,
    -64827, -64797, -64766, -64735, -64704, -64672, -64639, -64605, -64571, -64536, -64501, -64465,
    -64429, -64392, -64354, -64316, -64277, -64237, -64197, -64156, -64115, -64073, -64031, -63987,
    -63944, -63899, -63854, -63809, -63763, -63716, -63668, -63621, -63572, -63523, -63473, -63423,
    -63372, -63320, -63268, -63215, -63162, -63108, -63054, -62998, -62943, -62886, -62830, -62772,
    -62714, -62655, -62596, -62536, -62476, -62415, -62353, -62291, -62228, -62165, -62101, -62036,
    -61971, -61906, -61839, -61772, -61705, -61637, -61568, -61499, -61429, -61359, -61288, -61217,
    -61145, -61072, -60999, -60925, -60851, -60776, -60700, -60624, -60547, -60470, -60392, -60314,
    -60235, -60156, -60075, -59995, -59914, -59832, -59750, -59667, -59583, -59499, -59415, -59330,
    -59244, -59158, -59071, -58983, -58896, -58807, -58718, -58628, -58538, -58448, -58356, -58265,
    -58172, -58079, -57986, -57892, -57798, -57703, -57607, -57511, -57414, -57317, -57219, -57121,
    -57022, -56923, -56823, -56722, -56621, -56520, -56418, -56315, -56212, -56108, -56004, -55900,
    -55794, -55689, -55582, -55476, -55368, -55260, -55152, -55043, -54934, -54824, -54714, -54603,
    -54491, -54379, -54267, -54154, -54040, -53926, -53812, -53697, -53581, -53465, -53349, -53232,
    -53114, -52996, -52878, -52759, -52639, -52519, -52398, -52277, -52156, -52034, -51911, -51789,
    -51665, -51541, -51417, -51292, -51166, -51041, -50914, -50787, -50660, -50532, -50404, -50275,
    -50146, -50016, -49886, -49756, -49624, -49493, -49361, -49228, -49095, -48962, -48828, -48694,
    -48559, -48424, -48288, -48152, -48015, -47878, -47741, -47603, -47464, -47325, -47186, -47046,
    -46906, -46765, -46624, -46483, -46341, -46199, -46056, -45912, -45769, -45625, -45480, -45335,
    -45190, -45044, -44898, -44751, -44604, -44456, -44308, -44160, -44011, -43862, -43713, -43562,
    -43412, -43261, -43110, -42958, -42806, -42654, -42501, -42348, -42194, -42040, -41886, -41731,
    -41576, -41420, -41264, -41108, -40951, -40794, -40636, -40478, -40320, -40161, -40002, -39843,
    -39683, -39523, -39362, -39201, -39040, -38878, -38716, -38554, -38391, -38228, -38064, -37900,
    -37736, -37572, -37407, -37241, -37076, -36910, -36744, -36577, -36410, -36243, -36075, -35907,
    -35738, -35570, -35401, -35231, -35062, -34892, -34721, -34551, -34380, -34208, -34037, -33865,
    -33692, -33520, -33347, -33173, -33000, -32826, -32652, -32477, -32303, -32127, -31952, -31776,
    -31600, -31424, -31248, -31071, -30893, -30716, -30538, -30360, -30182, -30003, -29824, -29645,
    -29466, -29286, -29106, -28926, -28745, -28564, -28383, -28202, -28020, -27838, -27656, -27474,
    -27291, -27108, -26925, -26742, -26558, -26374, -26190, -26005, -25821, -25636, -25451, -25265,
    -25080, -24894, -24708, -24521, -24335, -24148, -23961, -23774, -23586, -23398, -23210, -23022,
    -22834, -22645, -22457, -22268, -22078, -21889, -21699, -21510, -21320, -21129, -20939, -20748,
    -20557, -20366, -20175, -19984, -19792, -19600, -19409, -19216, -19024, -18832, -18639, -18446,
    -18253, -18060, -17867, -17673, -17479, -17285, -17091, -16897, -16703, -16508, -16314, -16119,
    -15924, -15729, -15534, -15338, -15143, -14947, -14751, -14555, -14359, -14163, -13966, -13770,
    -13573, -13376, -13180, -12983, -12785, -12588, -12391, -12193, -11996, -11798, -11600, -11402,
    -11204, -11006, -10808, -10609, -10411, -10212, -10014, -9815, -9616, -9417, -9218, -9019,
    -8820, -8621, -8421, -8222, -8022, -7823, -7623, -7423, -7224, -7024, -6824, -6624,
    -6424, -6224, -6023, -5823, -5623, -5422, -5222, -5022, -4821, -4621, -4420, -4219,
    -4019, -3818, -3617, -3417, -3216, -3015, -2814, -2613, -2412, -2211, -2010, -1809,
    -1608, -1407, -1206, -1005, -804, -603, -402, -201, 0, 201, 402, 603,
    804, 1005, 1206, 1407, 1608, 1809, 2010, 2211, 2412, 2613, 2814, 3015,
    3216, 3417, 3617, 3818, 4019, 4219, 4420, 4621, 4821, 5022, 5222, 5422,
    5623, 5823, 6023, 6224, 6424, 6624, 6824, 7024, 7224, 7423, 7623, 7823,
    8022, 8222, 8421, 8621, 8820, 9019, 9218, 9417, 9616, 9815, 10014, 10212,
    10411, 10609, 10808, 11006, 11204, 11402, 11600, 11798, 11996, 12193, 12391, 12588,
    12785, 12983, 13180, 13376, 13573, 13770, 13966, 14163, 14359, 14555, 14751, 14947,
    15143, 15338, 15534, 15729, 15924, 16119, 16314, 16508, 16703, 16897, 17091, 17285,
    17479, 17673, 17867, 18060, 18253, 18446, 18639, 18832, 19024, 19216, 19409, 19600,
    19792, 19984, 20175, 20366, 20557, 20748, 20939, 21129, 21320, 21510, 21699, 21889,
    22078, 22268, 22457, 22645, 22834, 23022, 23210, 23398, 23586, 23774, 23961, 24148,
    24335, 24521, 24708, 24894, 25080, 25265, 25451, 25636, 25821, 26005, 26190, 26374,
    26558, 26742, 26925, 27108, 27291, 27474, 27656, 27838, 28020, 28202, 28383, 28564,
    28745, 28926, 29106, 29286, 29466, 29645, 29824, 30003, 30182, 30360, 30538, 30716,
    30893, 31071, 31248, 31424, 31600, 31776, 31952, 32127, 32303, 32477, 32652, 32826,
    33000, 33173, 33347, 33520, 33692, 33865, 34037, 34208, 34380, 34551, 34721, 34892,
    35062, 35231, 35401, 35570, 35738, 35907, 36075, 36243, 36410, 36577, 36744, 36910,
    37076, 37241, 37407, 37572, 37736, 37900, 38064, 38228, 38391, 38554, 38716, 38878,
    39040, 39201, 39362, 39523, 39683, 39843, 40002, 40161, 40320, 40478, 40636, 40794,
    40951, 41108, 41264, 41420, 41576, 41731, 41886, 42040, 42194, 42348, 42501, 42654,
    42806, 42958, 43110, 43261, 43412, 43562, 43713, 43862, 44011, 44160, 44308, 44456,
    44604, 44751, 44898, 45044, 45190, 45335, 45480, 45625, 45769, 45912, 46056, 46199,
    46341, 46483, 46624, 46765, 46906, 47046, 47186, 47325, 47464, 47603, 47741, 47878,
    48015, 48152, 48288, 48424, 48559, 48694, 48828, 48962, 49095, 49228, 49361, 49493,
    49624, 49756, 49886, 50016, 50146, 50275, 50404, 50532, 50660, 50787, 50914, 51041,
    51166, 51292, 51417, 51541, 51665, 51789, 51911, 52034, 52156, 52277, 52398, 52519,
    52639, 52759, 52878, 52996, 53114, 53232, 53349, 53465, 53581, 53697, 53812, 53926,
    54040, 54154, 54267, 54379, 54491, 54603, 54714, 54824, 54934, 55043, 55152, 55260,
    55368, 55476, 55582, 55689, 55794, 55900, 56004, 56108, 56212, 56315, 56418, 56520,
    56621, 56722, 56823, 56923, 57022, 57121, 57219, 57317, 57414, 57511, 57607, 57703,
    57798, 57892, 57986, 58079, 58172, 58265, 58356, 58448, 58538, 58628, 58718, 58807,
    58896, 58983, 59071, 59158, 59244, 59330, 59415, 59499, 59583, 59667, 59750, 59832,
    59914, 59995, 60075, 60156, 60235, 60314, 60392, 60470, 60547, 60624, 60700, 60776,
    60851, 60925, 60999, 61072, 61145, 61217, 61288, 61359, 61429, 61499, 61568, 61637,
    61705, 61772, 61839, 61906, 61971, 62036, 62101, 62165, 62228, 62291, 62353, 62415,
    62476, 62536, 62596, 62655, 62714, 62772, 62830, 62886, 62943, 62998, 63054, 63108,
    63162, 63215, 63268, 63320, 63372, 63423, 63473, 63523, 63572, 63621, 63668, 63716,
    63763, 63809, 63854, 63899, 63944, 63987, 64031, 64073, 64115, 64156, 64197, 64237,
    64277, 64316, 64354, 64392, 64429, 64465, 64501, 64536, 64571, 64605, 64639, 64672,
    64704, 64735, 64766, 64797, 64827, 64856, 64884, 64912, 64940, 64967, 64993, 65018,
    65043, 65067, 65091, 65114, 65137, 65159, 65180, 65200, 65220, 65240, 65259, 65277,
    65294, 65311, 65328, 65343, 65358, 65373, 65387, 65400, 65413, 65425, 65436, 65447,
    65457, 65467, 65476, 65484, 65492, 65499, 65505, 65511, 65516, 65521, 65525, 65528,
    65531, 65533, 65535, 65536,
};
SVECTOR gCellPositions[24] = {
    { -266, 99, -644, 0 },
    { -644, 99, -266, 0 },
    { -644, 99, 266, 0 },
    { -266, 99, 644, 0 },
    { 266, 99, 644, 0 },
    { 644, 99, 266, 0 },
    { 644, 99, -266, 0 },
    { 266, 99, -644, 0 },
    { -266, -99, -644, 0 },
    { -644, -99, -266, 0 },
    { -644, -99, 266, 0 },
    { -266, -99, 644, 0 },
    { 266, -99, 644, 0 },
    { 644, -99, 266, 0 },
    { 644, -99, -266, 0 },
    { 266, -99, -644, 0 },
    { -266, -299, -644, 0 },
    { -644, -299, -266, 0 },
    { -644, -299, 266, 0 },
    { -266, -299, 644, 0 },
    { 266, -299, 644, 0 },
    { 644, -299, 266, 0 },
    { 644, -299, -266, 0 },
    { 266, -299, -644, 0 },
};
SVECTOR gFlatCellPoints[8] = {
    { -99, 49, 0, 0 },
    { -41, -21, 0, 0 },
    { -99, -49, 0, 0 },
    { -41, 21, 0, 0 },
    { 99, 49, 0, 0 },
    { 41, -21, 0, 0 },
    { 99, -49, 0, 0 },
    { 41, 21, 0, 0 },
};
SVECTOR gCubeCellPoints[8] = {
    { -35, 35, -35, 0 },
    { -35, -35, 35, 0 },
    { -35, -35, -35, 0 },
    { -35, 35, 35, 0 },
    { 35, 35, -35, 0 },
    { 35, -35, 35, 0 },
    { 35, -35, -35, 0 },
    { 35, 35, 35, 0 },
};
SVECTOR gCollapsedCellPoints[8] = {
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

SVECTOR gRingPoints[64];
