/* Effects in the grid screen's 3D scene: the ring that rises, drops, tilts, ripples and
 * morphs, its burst into 32 pieces, the textured quad that orbits and then expands, and
 * the horizon sprites with 100 streaks. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>

#include "graphics.h"
#include "grid_effects.h"
#include "grid_nodes.h"
#include "grid_scene.h"
#include "media_cube.h"
#include "clip.h"
#include "pad.h"
#include "sound.h"
#include "ring_tmd.h"
#include "vertical_bars.h"


int gRingMotion;
int gRingRippleMode;

extern u_long gRingTimFlags[];

int gBurstFrame;
enum BurstState {
    BURST_IDLE,
    BURST_RISE,
    BURST_WAIT_FIRST_PULSE,
    BURST_FIRST_PULSE,
    BURST_WAIT_SECOND_PULSE,
    BURST_SECOND_PULSE,
    BURST_WAIT_SCATTER,
    BURST_SCATTER
};
int gBurstState;

extern u_long gRingTim[];
int gRingRipplePhase;
int gRingRippleShift;
/* gRingUnusedShort: 2 bytes; zeroed in InitRing, nothing reads it. */
short gRingUnusedShort, gRingTiltDegrees;
enum RingTilt {
    RING_TILT_NONE,
    RING_TILT_PITCH_POS,
    RING_TILT_PITCH_NEG,
    RING_TILT_ROLL_NEG,
    RING_TILT_ROLL_POS,
    RING_TILT_SETTLE_PITCH,
    RING_TILT_SETTLE_ROLL
};
int gRingTilt;

void InitRing(void)
{
    char unused[40]; /* 40 bytes; nothing uses it */

    gRingRippleShift = 4;
    gRingRipplePhase = 128;
    gRingTiltDegrees = 0;
    gRingUnusedShort = 0;
    gRingTilt = RING_TILT_NONE;
    gRingMotion = RING_MOTION_RISE_OVERSHOOT;
    InitModelCoordinate(&gModels[7]);
    SetModelTmd(gRingTmd, &gModels[7]);
    LinkModelObjects(&gModels[7], &gModels[1].coord);
    gModels[7].object.attribute = GsDOFF | GsALON | GsAONE;
    gModels[7].coord.coord.t[0] = gRingRestPosition.vx;
    gModels[7].coord.coord.t[1] = gRingRestPosition.vy + 900;
    gModels[7].coord.coord.t[2] = -gView.vpz + gRingRestPosition.vz;
    UploadTim(gRingTim);
    CopyPoints(gRingPoints, gRingFlatPoints, 64);
}

void InitPieceBurst(void)
{
    unsigned int i;

    gBurstState = BURST_RISE;
    gGridBusy = 255;
    gBurstFrame = 0;
    for (i = 0; i < 32; i++) {
        InitModelCoordinate(&gModels[i + 8]);
        SetModelTmd(gBurstPieceTmds[i], &gModels[i + 8]);
        LinkModelObjects(&gModels[i + 8], &gModels[1].coord);
        gModels[i + 8].coord.coord.t[0] = gRingRestPosition.vx;
        gModels[i + 8].coord.coord.t[1] = gRingRestPosition.vy;
        gModels[i + 8].coord.coord.t[2] = -gView.vpz + gRingRestPosition.vz;
        gModels[i + 8].object.attribute = GsALON | GsAONE;
    }
}

int gRingDipRollAt;
int gRingRippleDecayAt;

void StepRing(void)
{
    struct TMD_STRUCT object;
    unsigned char unused[8]; /* 8 bytes; nothing uses it */
    SVECTOR firstVertex, pairedVertex;
    unsigned int i;
    short phase;
    unsigned int now;

    if (gViewMode == VIEW_TOP || gViewMode == VIEW_DESCENT) {
        return;
    }
    gModels[7].object.attribute &= ~GsDOFF;
    RotateRingTimColumnsRight();
    now = gVSyncCount;
    if (now - gRingDipRollAt >= 3600) {
        gRingDipRollAt = now;
        if (gPointDeltaMode == POINT_DELTA_NONE && (rand() & 0x3000) && gSceneAnimation == SCENE_CMD_NONE && gRingMotion == RING_MOTION_IDLE) {
            StartPointMorph(gRingPoints, POINT_DELTA_RING_DIP, 20);
        }
    }
    switch (gRingMotion) {
    case RING_MOTION_LEVEL_UP:
        gModels[7].coord.coord.t[1] += 100;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_MOTION_RISE_OVERSHOOT:
        if (gRingTilt == RING_TILT_NONE) {
            gRingTilt = RING_TILT_PITCH_POS;
        }
        if (gRingRiseOvershoot.vy < gModels[7].coord.coord.t[1]) {
            gModels[7].coord.coord.t[1] -= 100;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingMotion = RING_MOTION_SETTLE_FROM_ABOVE;
            if (gRingSettleSoundPending != 0) {
                PlaySound(10);
                gRingSettleSoundPending = 0;
            }
        }
        break;
    case RING_MOTION_SETTLE_FROM_ABOVE:
        if (gRingRestPosition.vy > gModels[7].coord.coord.t[1]) {
            gModels[7].coord.coord.t[1] += 10;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingTilt = RING_TILT_SETTLE_PITCH;
            gRingMotion = RING_MOTION_IDLE;
        }
        break;
    case RING_MOTION_LEVEL_DOWN:
        gModels[7].coord.coord.t[1] -= 75;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_MOTION_DROP_OVERSHOOT:
        if (gRingTilt == RING_TILT_NONE) {
            gRingTilt = RING_TILT_PITCH_NEG;
        }
        if (gModels[7].coord.coord.t[1] < gRingDropOvershoot.vy) {
            gModels[7].coord.coord.t[1] += 75;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingMotion = RING_MOTION_SETTLE_FROM_BELOW;
        }
        break;
    case RING_MOTION_SETTLE_FROM_BELOW:
        if (gRingRestPosition.vy < gModels[7].coord.coord.t[1]) {
            gModels[7].coord.coord.t[1] -= 10;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingTilt = RING_TILT_SETTLE_PITCH;
            gRingMotion = RING_MOTION_IDLE;
        }
        break;
    case RING_MOTION_TURN_LEFT:
        gModels[7].coord.coord.t[0] += 6;
        gModels[7].coord.coord.t[1]++;
        gModels[7].coord.coord.t[2] += 6;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_MOTION_RETURN_FROM_LEFT:
        if (gRingTilt == RING_TILT_NONE) {
            gRingTilt = RING_TILT_ROLL_NEG;
        }
        if (gRingRestPosition.vx < gModels[7].coord.coord.t[0]) {
            gRingRippleMode = RING_RIPPLE_SCATTER;
            gModels[7].coord.coord.t[0] -= 6;
            gModels[7].coord.coord.t[1]--;
            gModels[7].coord.coord.t[2] -= 6;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingTilt = RING_TILT_SETTLE_ROLL;
            gRingMotion = RING_MOTION_IDLE;
        }
        break;
    case RING_MOTION_TURN_RIGHT:
        gModels[7].coord.coord.t[0] -= 6;
        gModels[7].coord.coord.t[1]++;
        gModels[7].coord.coord.t[2] += 6;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_MOTION_RETURN_FROM_RIGHT:
        if (gRingTilt == RING_TILT_NONE) {
            gRingTilt = RING_TILT_ROLL_POS;
        }
        if (gRingRestPosition.vx > gModels[7].coord.coord.t[0]) {
            gRingRippleMode = RING_RIPPLE_SCATTER;
            gModels[7].coord.coord.t[0] += 6;
            gModels[7].coord.coord.t[1]--;
            gModels[7].coord.coord.t[2] -= 6;
            UpdateModelMatrix(&gModels[7]);
        } else {
            gRingTilt = RING_TILT_SETTLE_ROLL;
            gRingMotion = RING_MOTION_IDLE;
        }
        break;
    default:
        gRingMotion = RING_MOTION_IDLE;
        break;
    }
    switch (gRingTilt) {
    case RING_TILT_PITCH_POS:
        if (gRingTiltDegrees < 8) gRingTiltDegrees += 2;
        else gRingTilt = RING_TILT_PITCH_NEG;
        gModels[7].rot.vx = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_TILT_PITCH_NEG:
        if (gRingTiltDegrees >= -7) gRingTiltDegrees -= 2;
        else gRingTilt = RING_TILT_PITCH_POS;
        gModels[7].rot.vx = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_TILT_ROLL_NEG:
        if (gRingTiltDegrees >= -4) gRingTiltDegrees--;
        else gRingTilt = RING_TILT_ROLL_POS;
        gModels[7].rot.vz = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_TILT_ROLL_POS:
        if (gRingTiltDegrees < 5) gRingTiltDegrees++;
        else gRingTilt = RING_TILT_ROLL_NEG;
        gModels[7].rot.vz = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_TILT_SETTLE_PITCH:
        if (gRingTiltDegrees > 0) gRingTiltDegrees--;
        else if (gRingTiltDegrees < 0) gRingTiltDegrees++;
        else if (gRingTiltDegrees == 0) {
            gRingTilt = RING_TILT_NONE;
            gRingRippleMode = RING_RIPPLE_WAVE;
        }
        gModels[7].rot.vx = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    case RING_TILT_SETTLE_ROLL:
        if (gRingTiltDegrees > 0) gRingTiltDegrees--;
        else if (gRingTiltDegrees < 0) gRingTiltDegrees++;
        else if (gRingTiltDegrees == 0) {
            gRingTilt = RING_TILT_NONE;
            gRingRippleMode = RING_RIPPLE_WAVE;
        }
        gModels[7].rot.vz = (gRingTiltDegrees << 12) / 360;
        UpdateModelMatrix(&gModels[7]);
        break;
    default:
        gRingTilt = RING_TILT_NONE;
        break;
    }
    if (gPointDeltaStepsLeft > 0) {
        if (gPointDeltaMode != POINT_DELTA_CELL_FLIGHT) {
            ApplyPointDeltas(gRingPoints);
        }
    } else if (gPointDeltaMode == POINT_DELTA_RING_DIP) {
        StartPointMorph(gRingPoints, POINT_DELTA_RING_FLAT, 20);
    } else {
        if (gPointDeltaMode == POINT_DELTA_RING_FLAT) {
            CopyPoints(gRingPoints, gRingFlatPoints, 64);
        }
        gPointDeltaMode = POINT_DELTA_NONE;
    }
    object = *gModels[7].objects;
    gRingRipplePhase++;
    for (i = 0; i < object.vern / 2; phase = rand() & 127) {
        switch (gRingRippleMode) {
        case RING_RIPPLE_WAVE: {
            int sample = (u_char)gSineTable[((i + gRingRipplePhase) >> 2) & 0x7ff] >> gRingRippleShift;
            firstVertex.vx = gRingPoints[i].vx;
            pairedVertex.vx = gRingPoints[i + object.vern / 2].vx;
            firstVertex.vy = gRingPoints[i].vy + sample;
            pairedVertex.vy = gRingPoints[i + object.vern / 2].vy + sample;
            firstVertex.vz = gRingPoints[i].vz;
            pairedVertex.vz = gRingPoints[i + object.vern / 2].vz;
            break;
        }
        case RING_RIPPLE_SCATTER: {
            int sample = (u_char)gSineTable[(phase + gRingRipplePhase) & 0x7ff] >> 4;
            firstVertex.vx = gRingPoints[i].vx + sample;
            pairedVertex.vx = gRingPoints[i + object.vern / 2].vx + sample;
            firstVertex.vy = gRingPoints[i].vy + sample;
            pairedVertex.vy = gRingPoints[i + object.vern / 2].vy + sample;
            firstVertex.vz = gRingPoints[i].vz + sample;
            pairedVertex.vz = gRingPoints[i + object.vern / 2].vz + sample;
            break;
        }
        default:
            firstVertex.vx = gRingPoints[i].vx;
            pairedVertex.vx = gRingPoints[i + object.vern / 2].vx;
            firstVertex.vy = gRingPoints[i].vy;
            pairedVertex.vy = gRingPoints[i + object.vern / 2].vy;
            gRingRipplePhase = 128;
            firstVertex.vz = gRingPoints[i].vz;
            pairedVertex.vz = gRingPoints[i + object.vern / 2].vz;
            gRingRippleShift = 16;
            break;
        }
        SetTmdVertex(i, &firstVertex, &object);
        SetTmdVertex(i + object.vern / 2, &pairedVertex, &object);
        i++;
    }
    if (gRingRippleMode == RING_RIPPLE_WAVE) {
        if (gRingRippleShift >= 5) {
            now = gVSyncCount;
            if (now - gRingRippleDecayAt >= 30) {
                gRingRippleDecayAt = now;
                gRingRippleShift--;
            }
        }
        if (gRingRipplePhase >= 1409) {
            gRingRipplePhase = 128;
        }
    }
    SortModel(&gModels[7], gOt[gActiveBuffer]);
}

enum BurstPulse {
    BURST_PULSE_HOLD,
    BURST_PULSE_OUT,
    BURST_PULSE_IN = 255
};
int gBurstPulse;

void StepPieceBurst(void)
{
    MATRIX rot;
    SVECTOR step, angles;
    VECTOR delta;
    VECTOR unused; /* 16 bytes; nothing uses it */
    unsigned int i;

    if (gBurstFrame < 40) {
        RotateRingTimColumnsRight();
    }
    if (gBurstFrame == 51) {
        PlaySound(23);
    }
    for (i = 0; i < 32; i++) {
        gModels[i + 8].object.attribute &= ~GsDOFF;
        switch (gBurstState) {
        case BURST_RISE:
            if (gBurstFrame < 15) {
                gModels[i + 8].coord.coord.t[1] -= 18;
                UpdateModelMatrix(&gModels[i + 8]);
            } else {
                gBurstState = BURST_WAIT_FIRST_PULSE;
            }
            break;
        case BURST_WAIT_FIRST_PULSE:
            if (gBurstFrame >= 18) {
                gBurstState = BURST_FIRST_PULSE;
                gBurstPulse = BURST_PULSE_HOLD;
            }
            break;
        case BURST_FIRST_PULSE:
            PushMatrix();
            angles.vy = (short)((i * 11.25 * 4096.0) / 360.0);
            angles.vx = 0;
            angles.vz = 0;
            RotMatrix(&angles, &rot);
            SetRotMatrix(&rot);
            step.vx = 0;
            step.vy = 0;
            step.vz = 50;
            ApplyRotMatrix(&step, &delta);
            PopMatrix();
            if (gBurstPulse == BURST_PULSE_OUT) {
                gModels[i + 8].coord.coord.t[0] += delta.vx;
                gModels[i + 8].coord.coord.t[1] += delta.vy;
                gModels[i + 8].coord.coord.t[2] += delta.vz;
            } else if (gBurstPulse == BURST_PULSE_IN) {
                gModels[i + 8].coord.coord.t[0] -= delta.vx;
                gModels[i + 8].coord.coord.t[1] -= delta.vy;
                gModels[i + 8].coord.coord.t[2] -= delta.vz;
            }
            UpdateModelMatrix(&gModels[i + 8]);
            break;
        case BURST_WAIT_SECOND_PULSE:
            if (gBurstFrame >= 28) {
                gBurstState = BURST_SECOND_PULSE;
                gBurstPulse = BURST_PULSE_HOLD;
            }
            break;
        case BURST_SECOND_PULSE:
            PushMatrix();
            angles.vy = (short)((i * 11.25 * 4096.0) / 360.0);
            angles.vx = 0;
            angles.vz = 0;
            RotMatrix(&angles, &rot);
            SetRotMatrix(&rot);
            step.vx = 0;
            step.vy = 0;
            step.vz = 50;
            ApplyRotMatrix(&step, &delta);
            PopMatrix();
            if (gBurstPulse == BURST_PULSE_OUT) {
                gModels[i + 8].coord.coord.t[0] += delta.vx;
                gModels[i + 8].coord.coord.t[1] += delta.vy;
                gModels[i + 8].coord.coord.t[2] += delta.vz;
            } else if (gBurstPulse == BURST_PULSE_IN) {
                gModels[i + 8].coord.coord.t[0] -= delta.vx;
                gModels[i + 8].coord.coord.t[1] -= delta.vy;
                gModels[i + 8].coord.coord.t[2] -= delta.vz;
            }
            UpdateModelMatrix(&gModels[i + 8]);
            break;
        case BURST_WAIT_SCATTER:
            if (gBurstFrame >= 40) {
                gBurstState = BURST_SCATTER;
                gRingTiltDegrees = 0;
            }
            break;
        case BURST_SCATTER:
            if (gBurstFrame < 43) {
                if (gRingTiltDegrees < 15) {
                    gRingTiltDegrees = (short)(gRingTiltDegrees + 0.5);
                }
                gModels[i + 8].rot.vx = (gRingTiltDegrees << 12) / 360;
                UpdateModelMatrix(&gModels[i + 8]);
            } else if (gBurstFrame < 51) {
                PushMatrix();
                angles.vy = (short)((i * 11.25 * 4096.0) / 360.0);
                angles.vx = 0;
                angles.vz = 0;
                RotMatrix(&angles, &rot);
                SetRotMatrix(&rot);
                step.vx = 0;
                step.vy = 0;
                step.vz = 100;
                ApplyRotMatrix(&step, &delta);
                PopMatrix();
                gModels[i + 8].coord.coord.t[0] += delta.vx;
                gModels[i + 8].coord.coord.t[1] += delta.vy;
                gModels[i + 8].coord.coord.t[2] += delta.vz;
                UpdateModelMatrix(&gModels[i + 8]);
            } else {
                gSceneAnimation = SCENE_CMD_NONE;
                gSceneCommand = SCENE_CMD_NONE;
                gGridBusy = 0;
                gBurstState = BURST_IDLE;
                gBurstFrame = 0;
                gModels[i + 8].object.attribute |= GsDOFF;
            }
            break;
        default:
            gSceneAnimation = SCENE_CMD_NONE;
            gSceneCommand = SCENE_CMD_NONE;
            gGridBusy = 0;
            break;
        }
        SortModel(&gModels[i + 8], gOt[gActiveBuffer]);
    }

    if (gBurstState != BURST_IDLE) {
        gBurstFrame++;
    }
    if (gBurstState == BURST_FIRST_PULSE || gBurstState == BURST_SECOND_PULSE) {
        if (gBurstPulse == BURST_PULSE_HOLD) {
            gBurstPulse = BURST_PULSE_OUT;
        } else if (gBurstPulse == BURST_PULSE_OUT) {
            gBurstPulse = BURST_PULSE_IN;
        } else if (gBurstPulse == BURST_PULSE_IN) {
            if (gBurstState == BURST_FIRST_PULSE) {
                gBurstState = BURST_WAIT_SECOND_PULSE;
            } else if (gBurstState == BURST_SECOND_PULSE) {
                gBurstState = BURST_WAIT_SCATTER;
            }
        }
    }
    if (gBurstState == BURST_IDLE) {
        InitRing();
    }
}

void RotateRingTimColumnsRight(void)
{
    GsIMAGE tim;
    RECT rect;

    GsGetTimInfo(gRingTimFlags, &tim);
    rect.x = tim.px;
    rect.y = tim.py;
    rect.w = tim.pw;
    rect.h = tim.ph;
    MoveImage(&rect, tim.px + tim.pw, tim.py);
    DrawSync(0);
    rect.x = tim.px + tim.pw - 1;
    rect.y = tim.py;
    rect.w = tim.pw;
    rect.h = tim.ph;
    MoveImage(&rect, tim.px + tim.pw, tim.py);
    DrawSync(0);
    rect.x = tim.px + tim.pw;
    rect.y = tim.py;
    rect.w = tim.pw;
    rect.h = tim.ph;
    MoveImage(&rect, tim.px, tim.py);
    DrawSync(0);
}

void CopyPoints(SVECTOR *dst, const SVECTOR *src, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}

extern u_long gHorizonTim[];

extern u_long gHorizonTimFlags[];
GsSPRITE gHorizonSprites[10];
Streak gStreaks[100];
int gStreakUnusedWord; /* 4 bytes; zeroed in InitHorizonAndStreaks, nothing reads it. */
u_char gPathStreaksReady;
u_char gStreakUnusedByte; /* 1 byte; zeroed in InitHorizonAndStreaks, nothing reads it. */

int gQuadAngle;
typedef struct {
    short halfWidth;
    short halfHeight;
} QuadHalfExtents;

QuadHalfExtents gQuadHalfExtents;
SVECTOR gSavedQuadOrbitPoint; /* 8 bytes; set from gQuadOrbitPoint when idle, nothing reads it. */
SVECTOR gQuadPosition;
POLY_FT4 gTexturedQuad;

void StartPointMorph(const SVECTOR *points, unsigned int mode, int steps)
{
    SVECTOR *target;

    switch (mode) {
    case POINT_DELTA_RING_BENT_DOWN:
        target = gRingBentDownPoints;
        break;
    case POINT_DELTA_RING_BENT_UP:
        target = gRingBentUpPoints;
        break;
    case POINT_DELTA_RING_DIP:
        target = gRingDipPoints;
        break;
    case POINT_DELTA_RING_FLAT:
        target = gRingFlatPoints;
        break;
    default:
        target = gRingFlatPoints;
        steps = 10;
        break;
    }
    gPointDeltaMode = mode;
    InitPointDeltas(target, points, (short)(steps - 1), 64);
}

extern u_long gQuadTim[];
extern u_long gQuadTimFlags[];

void InitTexturedQuad(void)
{
    GsIMAGE tim;
    POLY_FT4 *poly;

    UploadTim(gQuadTim);
    GsGetTimInfo(gQuadTimFlags, &tim);
    poly = &gTexturedQuad;
    poly->r0 = poly->g0 = poly->b0 = 128;
    poly->tpage = GetTPage(tim.pmode, 1, tim.px, tim.py);
    poly->clut = GetClut(tim.cx, tim.cy);
    setXY4(poly, 0, 0, 0, 0, 0, 0, 0, 0);
    gQuadAngle = 0;
    gQuadHalfExtents.halfWidth = (u_short)tim.pw - 1;
    gQuadHalfExtents.halfHeight = ((u_short)tim.ph >> 1) - 1;
    poly->u0 = tim.px;
    poly->v0 = tim.py;
    poly->u1 = tim.px + tim.pw * 2 - 1;
    poly->v1 = tim.py;
    poly->u2 = tim.px;
    poly->v2 = tim.py + tim.ph - 1;
    poly->u3 = tim.px + tim.pw * 2 - 1;
    poly->v3 = tim.py + tim.ph - 1;
}

void DrawOrbitingTexturedQuad(void)
{
    MATRIX matrix;
    MATRIX unused[2]; /* 64 bytes; nothing uses it */
    SVECTOR modelPoint;
    SVECTOR rotation;
    VECTOR rotated;
    VECTOR translation;
    u_int brightness;

    if (gViewMode == VIEW_FRONT && gSceneAnimation != SCENE_CMD_QUAD_EXPAND) {
        setPolyFT4(&gTexturedQuad);
        SetSemiTrans(&gTexturedQuad, 1);
        brightness = rand() & 0xff;
        if (brightness < 128) {
            brightness = 128;
        }
        gTexturedQuad.b0 = brightness;
        gTexturedQuad.g0 = brightness;
        gTexturedQuad.r0 = brightness;
        PushMatrix();
        rotation.vy = 0;
        rotation.vx = (gQuadAngle << 12) / 360;
        rotation.vz = (gQuadAngle << 13) / 360;
        RotMatrix(&rotation, &matrix);
        translation.vx = 0;
        translation.vy = 0;
        translation.vz = 0;
        TransMatrix(&matrix, &translation);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        modelPoint.vx = gQuadOrbitPoint.vx;
        modelPoint.vy = gQuadOrbitPoint.vy;
        modelPoint.vz = gQuadOrbitPoint.vz;
        ApplyMatrix(&matrix, &modelPoint, &rotated);
        PopMatrix();
        gQuadAngle += 2;
        if (gQuadAngle >= 360) {
            gQuadAngle = 0;
        }
        gQuadPosition.vx = rotated.vx + 160;
        gQuadPosition.vy = (rotated.vy + 120) + gTiltOffsetY;
        gQuadPosition.vz = rotated.vz;
        gTexturedQuad.x0 = gQuadPosition.vx - gQuadHalfExtents.halfWidth;
        gTexturedQuad.y0 = gQuadPosition.vy - gQuadHalfExtents.halfHeight;
        gTexturedQuad.x1 = gQuadPosition.vx + gQuadHalfExtents.halfWidth;
        gTexturedQuad.y1 = gQuadPosition.vy - gQuadHalfExtents.halfWidth;
        gTexturedQuad.x2 = gQuadPosition.vx - gQuadHalfExtents.halfHeight;
        gTexturedQuad.y2 = gQuadPosition.vy + gQuadHalfExtents.halfHeight;
        gTexturedQuad.x3 = gQuadPosition.vx + gQuadHalfExtents.halfWidth;
        gTexturedQuad.y3 = gQuadPosition.vy + gQuadHalfExtents.halfHeight;
        GsSortPoly(&gTexturedQuad, &gOt[gActiveBuffer], rotated.vz + 350);
        DrawSync(0);
        gFlatLights[2].g = 0x30;
        gFlatLights[2].r = 0x30;
        gFlatLights[2].vx = ~rotated.vx;
        gFlatLights[2].vy = ~rotated.vy;
        gFlatLights[2].vz = ~rotated.vz;
        GsSetFlatLight(2, &gFlatLights[2]);
    } else {
        gQuadAngle = 0;
        gSavedQuadOrbitPoint.vx = gQuadOrbitPoint.vx;
        gSavedQuadOrbitPoint.vy = gQuadOrbitPoint.vy;
        gSavedQuadOrbitPoint.vz = gQuadOrbitPoint.vz;
    }
}

int gQuadExpandFrames;
int gQuadGrowth;
short gQuadStepX;
short gQuadStepY;

void DrawExpandingTexturedQuad(void)
{
    POLY_FT4 *poly = &gTexturedQuad;
    unsigned int brightness;
    unsigned short x, y, depth;
    unsigned short halfWidth, halfHeight;

    setPolyFT4(poly);
    SetSemiTrans(poly, 1);
    brightness = rand() & 0xff;
    if (brightness < 128) {
        brightness = 128;
    }
    poly->b0 = brightness;
    poly->g0 = brightness;
    poly->r0 = brightness;
    if (gQuadExpandFrames > 0) {
        depth = gQuadPosition.vz;
        gQuadPosition.vz -= 4;
        x = gQuadPosition.vx + gQuadStepX;
        gQuadPosition.vx = x;
        gQuadGrowth += 4;
        y = gQuadPosition.vy + gQuadStepY;
        halfWidth = gQuadHalfExtents.halfWidth + gQuadGrowth;
        halfHeight = gQuadHalfExtents.halfHeight + gQuadGrowth;
        gQuadPosition.vy = y;
        poly->x0 = x - halfWidth;
        poly->y0 = y - halfHeight;
        poly->x1 = x + halfWidth;
        poly->y1 = y - halfWidth;
        poly->x2 = x - halfHeight;
        poly->y2 = y + halfHeight;
        poly->x3 = x + halfWidth;
        poly->y3 = y + halfHeight;
        GsSortPoly(poly, &gOt[gActiveBuffer], depth + 346);
        DrawSync(0);
        gFlatLights[2].g = 0x20;
        gFlatLights[2].r = 0x20;
        gFlatLights[2].vx = ~gQuadPosition.vx;
        gFlatLights[2].vy = ~gQuadPosition.vy;
        gFlatLights[2].vz = ~gQuadPosition.vz;
        GsSetFlatLight(2, &gFlatLights[2]);
        gQuadExpandFrames--;
    } else {
        InitTexturedQuad();
        gGridBusy = 0;
        gSceneAnimation = SCENE_CMD_NONE;
        gSceneCommand = SCENE_CMD_NONE;
    }
}

void StartExpandingTexturedQuad(void)
{
    unsigned char unused[32]; /* 32 bytes; nothing uses it */
    int dx;
    int dy;

    gGridBusy = 255;
    gQuadAngle = 0;
    gQuadGrowth = 0;
    if (gQuadPosition.vz >= 0) {
        gQuadPosition.vz = 50;
        gQuadExpandFrames = 20;
    } else {
        gQuadExpandFrames = 15;
    }
    dx = (160 - gQuadPosition.vx) / gQuadExpandFrames;
    dy = (120 - gQuadPosition.vy) / gQuadExpandFrames;
    gQuadStepX = dx;
    gQuadStepY = dy;
}

void InitHorizonAndStreaks(void)
{
    GsIMAGE image;
    u_int i;
    int x;

    x = 0;
    UploadTim(gHorizonTim);
    GsGetTimInfo(gHorizonTimFlags, &image);
    for (i = 0; i < 10; i++) {
        gHorizonSprites[i].attribute = 0;
        gHorizonSprites[i].x = x;
        gHorizonSprites[i].y = 88;
        gHorizonSprites[i].r = gHorizonSprites[i].g = gHorizonSprites[i].b = 192;
        gHorizonSprites[i].scalex = gHorizonSprites[i].scaley = 1;
        gHorizonSprites[i].rotate = 0;
        gHorizonSprites[i].tpage = GetTPage(image.pmode, 0, image.px, image.py);
        gHorizonSprites[i].u = 0;
        gHorizonSprites[i].v = image.py;
        gHorizonSprites[i].w = image.pw << 2;
        gHorizonSprites[i].h = image.ph;
        gHorizonSprites[i].cx = image.cx;
        gHorizonSprites[i].cy = image.cy;
        x += image.pw << 2;
    }
    for (i = 0; i < 100; i++) {
        InitVerticalStreak(&gStreaks[i]);
    }
    InitModelCoordinate(&gModels[6]);
    SetModelTmd(gVerticalBarsTmd, &gModels[6]);
    LinkModelObjects(&gModels[6], &gModels[1].coord);
    gModels[6].coord.coord.t[0] = 0;
    gModels[6].coord.coord.t[1] = 0;
    gModels[6].object.attribute = GsDOFF | GsALON | GsAONE;
    gModels[6].coord.coord.t[2] = -gView.vpz;
    gStreakUnusedWord = 0;
    gStreakUnusedByte = 0;
    gPathStreaksReady = 0;
}

void InitPathStreaks(void)
{
    unsigned int i;

    for (i = 0; i < 8; i++) {
        InitPathStreak(&gStreaks[i], gNearStreakPathNegXNegZ);
        InitPathStreak(&gStreaks[i + 8], gNearStreakPathPosXNegZ);
        InitPathStreak(&gStreaks[i + 16], gFarStreakPathNegXNegZ);
        InitPathStreak(&gStreaks[i + 24], gFarStreakPathPosXNegZ);
    }
    gPathStreaksReady = 0xff;
}

void StepHorizonAndStreaks(void)
{
    SVECTOR unusedRotation; /* 8 bytes; nothing uses it */
    unsigned int i;

    if (gView.vpy >= -2699 || gView.vpy == (-2147483647 - 1)) {
        if (gPathStreaksReady == 0) {
            InitPathStreaks();
        } else {
            DrawSync(0);
            for (i = 0; i < 10; i++) {
                if (gSceneAnimation == SCENE_CMD_TILT_UP || gSceneAnimation == SCENE_CMD_TILT_DOWN) {
                    gHorizonSprites[i].y = gTiltOffsetY + 88;
                } else if (gSceneAnimation == SCENE_CMD_VIEW_TO_TOP || gSceneAnimation == SCENE_CMD_VIEW_TO_FRONT ||
                           (gViewReady != 0 && gViewTiltedUp == 0 && gViewTiltedDown == 0)) {
                    gHorizonSprites[i].y = gView.vpy / 4 + 88;
                }
                GsSortFastSprite(&gHorizonSprites[i], &gOt[gActiveBuffer], 1050);
            }
            DrawSync(0);
            for (i = 0; i < 8; i++) {
                StepStreakNegXNegZ(&gStreaks[i]);
                StepStreakPosXNegZ(&gStreaks[i + 8]);
                StepStreakNegXNegZ(&gStreaks[i + 16]);
                StepStreakPosXNegZ(&gStreaks[i + 24]);
            }
            gModels[6].rot.vy += 45;
            UpdateModelMatrix(&gModels[6]);
            gModels[6].object.attribute &= ~GsDOFF;
            SortModel(&gModels[6], gOt[gActiveBuffer]);
        }
    }
    if (gViewMode == VIEW_DESCENT && gView.vpy != (-2147483647 - 1)) {
        DrawSync(0);
        if (gView.vpy < -2700) {
            for (i = 0; i < 100; i++) {
                StepVerticalStreak(&gStreaks[i]);
            }
        } else {
            for (i = 0; i < 50; i++) {
                StepVerticalStreak(&gStreaks[i + 32]);
            }
        }
    }
}

void InitVerticalStreak(Streak *rec)
{
    u_int k;
    short n;

    InitChildCoordinatePair(rec, &gModels[1].coord);
    rec->start.vx = gVerticalStreakXs[rand() & 0xf];
    rec->start.vy = 500;
    rec->start.vz = gVerticalStreakZs[rand() & 0xf];
    rec->head.coord.t[0] = rec->start.vx;
    rec->head.coord.t[1] = rec->start.vy;
    rec->head.coord.t[2] = rec->start.vz;
    rec->end.vx = rec->start.vx;
    rec->end.vy = -20000;
    rec->end.vz = rec->start.vz;
    rec->tail.coord.t[0] = rec->head.coord.t[0];
    rec->tail.coord.t[1] = rec->head.coord.t[1];
    rec->tail.coord.t[2] = rec->head.coord.t[2];
    UpdateCoordinatePairMatrices(rec);
    rec->line.attribute = GsALON | GsAONE;
    k = rand() & 0x300;
    rec->r = gStreakColors[k >> 8][0];
    rec->g = gStreakColors[k >> 8][1];
    rec->b = gStreakColors[k >> 8][2];
    rec->line.r0 = gStreakColors[k >> 8][0];
    rec->line.g0 = gStreakColors[k >> 8][1];
    rec->line.b0 = gStreakColors[k >> 8][2];
    rec->line.r1 = rec->line.g1 = rec->line.b1 = 0;
    n = rand() & 0x7ff;
    if (n < 500) {
        n = 500;
    }
    rec->speed = n;
    n = rand() & 0xfff;
    if (n < 2000) {
        n = 2000;
    }
    rec->length = n;
}

void InitPathStreak(Streak *rec, SVECTOR *path)
{
    u_char *color;
    short n;
    u_char (*palette)[3];
    u_int choice;

    InitChildCoordinatePair(rec, &gModels[1].coord);
    rec->start.vx = path[1].vx;
    rec->start.vy = gPathStreakHeights[rand() & 0xf];
    rec->start.vz = path[1].vz - gView.vpz;
    rec->end.vx = path[0].vx;
    rec->end.vy = path[0].vy;
    rec->end.vz = path[0].vz;
    rec->head.coord.t[0] = rec->start.vx;
    rec->head.coord.t[1] = rec->start.vy;
    rec->head.coord.t[2] = rec->start.vz;
    rec->tail.coord.t[0] = rec->head.coord.t[0];
    rec->tail.coord.t[1] = rec->head.coord.t[1];
    rec->tail.coord.t[2] = rec->head.coord.t[2];
    UpdateCoordinatePairMatrices(rec);
    rec->line.attribute = GsALON | GsAONE;
    choice = rand() & 0x300;
    palette = gStreakColors;
    color = palette[choice >> 8];
    rec->r = color[0];
    rec->g = color[1];
    rec->b = color[2];
    rec->line.r0 = color[0];
    rec->line.g0 = color[1];
    rec->line.b0 = color[2];
    rec->line.r1 = rec->line.g1 = rec->line.b1 = 0;
    n = rand() & 0x3ff;
    if (n < 200) {
        n = 200;
    }
    rec->speed = n;
    n = rand() & 0xfff;
    if (n < 2000) {
        n = 2000;
    }
    rec->length = n;
}

void StepStreakNegXNegZ(Streak *rec)
{
    int z;
    short speed;
    short length;
    short *from;
    u_char r, g, b;

    if (rec->end.vx < rec->head.coord.t[0] && rec->end.vz < rec->head.coord.t[2]) {
        rec->head.coord.t[0] -= rec->speed;
        rec->head.coord.t[2] -= rec->speed;
    } else {
        rec->head.coord.t[0] = rec->start.vx;
        rec->head.coord.t[1] = gPathStreakHeights[rand() & 15];
        r = rec->r;
        z = rec->start.vz - gView.vpz;
        b = rec->b;
        rec->line.r0 = r;
        g = rec->g;
        rec->line.g0 = g;
        rec->line.b0 = b;
        rec->head.coord.t[2] = z;
        speed = rand() & 1023;
        if (speed < 200) {
            speed = 200;
        }
        rec->speed = speed;
        length = rand() & 4095;
        if (length < 2000) {
            length = 2000;
        }
        rec->length = length;
    }
    rec->tail.coord.t[0] = rec->head.coord.t[0] + rec->length;
    rec->tail.coord.t[1] = rec->head.coord.t[1];
    rec->tail.coord.t[2] = rec->head.coord.t[2] + rec->length;
    UpdateCoordinatePairMatrices(rec);
    from = &rec->line.x0;
    ProjectCoordinatePairToScreen(&rec->head, from, &rec->line.x1);
    if (IsPointOffScreen(from, 100) == 0) {
        GsSortGLine(&rec->line, &gOt[gActiveBuffer], 700);
    }
}

void StepStreakPosXNegZ(Streak *rec)
{
    int random;
    int z;
    short speed;
    short length;
    short *from;
    u_char r, g, b;

    if (rec->head.coord.t[0] < rec->end.vx && rec->end.vz < rec->head.coord.t[2]) {
        rec->head.coord.t[0] += rec->speed;
        rec->head.coord.t[2] -= rec->speed;
    } else {
        rec->head.coord.t[0] = rec->start.vx;
        random = rand();
        rec->head.coord.t[1] = gPathStreakHeights[random & 15];
        z = rec->start.vz - gView.vpz;
        r = rec->r;
        g = rec->g;
        b = rec->b;
        rec->line.r0 = r;
        rec->line.g0 = g;
        rec->line.b0 = b;
        rec->head.coord.t[2] = z;
        speed = rand() & 1023;
        if (speed < 200) {
            speed = 200;
        }
        rec->speed = speed;
        length = rand() & 4095;
        if (length < 2000) {
            length = 2000;
        }
        rec->length = length;
    }
    rec->tail.coord.t[0] = rec->head.coord.t[0] - rec->length;
    rec->tail.coord.t[1] = rec->head.coord.t[1];
    rec->tail.coord.t[2] = rec->head.coord.t[2] + rec->length;
    UpdateCoordinatePairMatrices(rec);
    from = &rec->line.x0;
    ProjectCoordinatePairToScreen(&rec->head, from, &rec->line.x1);
    if (IsPointOffScreen(from, 100) == 0) {
        GsSortGLine(&rec->line, &gOt[gActiveBuffer], 700);
    }
}

void StepVerticalStreak(Streak *rec)
{
    short *from;
    short n;
    int random;
    int x;
    int z;
    u_char r;
    u_char g;
    u_char b;

    if (rec->end.vy < rec->head.coord.t[1]) {
        rec->head.coord.t[1] -= rec->speed;
    } else {
        random = rand();
        x = gVerticalStreakXs[random & 0xf];
        rec->head.coord.t[1] = 500;
        rec->head.coord.t[0] = x;
        random = rand();
        z = gVerticalStreakZs[random & 0xf];
        r = rec->r;
        g = rec->g;
        b = rec->b;
        rec->head.coord.t[2] = z;
        rec->line.r0 = r;
        rec->line.g0 = g;
        rec->line.b0 = b;
        rec->end.vy = gView.vpy;
        n = rand() & 0x7ff;
        if (n < 500) {
            n = 500;
        }
        rec->speed = n;
        n = rand() & 0xfff;
        if (n < 2000) {
            n = 2000;
        }
        rec->length = n;
    }
    rec->tail.coord.t[0] = rec->head.coord.t[0];
    rec->tail.coord.t[1] = rec->head.coord.t[1] + rec->length;
    rec->tail.coord.t[2] = rec->head.coord.t[2];
    UpdateCoordinatePairMatrices(rec);
    if (rec->head.coord.t[1] > gView.vpy) {
        from = &rec->line.x0;
        ProjectCoordinatePairToScreen(&rec->head, from, &rec->line.x1);
        if (IsPointOffScreen(from, 500) == 0) {
            GsSortGLine(&rec->line, &gOt[gActiveBuffer], 700);
        }
    }
}

SVECTOR gCubeCorners[8] = {
    { 29, -29, -29, 0 },
    { 29, 29, -29, 0 },
    { -29, -29, -29, 0 },
    { -29, 29, -29, 0 },
    { 29, -29, 29, 0 },
    { 29, 29, 29, 0 },
    { -29, -29, 29, 0 },
    { -29, 29, 29, 0 },
};
SVECTOR gCubePieceStarts[8] = {
    { 1024, -240, -1024, 0 },
    { -200, -500, -1024, 0 },
    { 1024, -240, -1024, 0 },
    { -200, -500, -1024, 0 },
    { 1024, -240, 1024, 0 },
    { -200, -500, 1024, 0 },
    { 1024, -240, 1024, 0 },
    { -200, -500, 1024, 0 },
};
SVECTOR gRingFlatPoints[64] = {
    { 375, 7, 0, 0 },
    { 367, 7, 73, 0 },
    { 346, 7, 143, 0 },
    { 311, 7, 208, 0 },
    { 265, 7, 265, 0 },
    { 208, 7, 311, 0 },
    { 143, 7, 346, 0 },
    { 73, 7, 367, 0 },
    { 0, 7, 375, 0 },
    { -73, 7, 367, 0 },
    { -143, 7, 346, 0 },
    { -208, 7, 311, 0 },
    { -265, 7, 265, 0 },
    { -311, 7, 208, 0 },
    { -346, 7, 143, 0 },
    { -367, 7, 73, 0 },
    { -375, 7, 0, 0 },
    { -367, 7, -73, 0 },
    { -346, 7, -143, 0 },
    { -311, 7, -208, 0 },
    { -265, 7, -265, 0 },
    { -208, 7, -311, 0 },
    { -143, 7, -346, 0 },
    { -73, 7, -367, 0 },
    { 0, 7, -375, 0 },
    { 73, 7, -367, 0 },
    { 143, 7, -346, 0 },
    { 208, 7, -311, 0 },
    { 265, 7, -265, 0 },
    { 311, 7, -208, 0 },
    { 346, 7, -143, 0 },
    { 367, 7, -73, 0 },
    { 375, -7, 0, 0 },
    { 367, -7, 73, 0 },
    { 346, -7, 143, 0 },
    { 311, -7, 208, 0 },
    { 265, -7, 265, 0 },
    { 208, -7, 311, 0 },
    { 143, -7, 346, 0 },
    { 73, -7, 367, 0 },
    { 0, -7, 375, 0 },
    { -73, -7, 367, 0 },
    { -143, -7, 346, 0 },
    { -208, -7, 311, 0 },
    { -265, -7, 265, 0 },
    { -311, -7, 208, 0 },
    { -346, -7, 143, 0 },
    { -367, -7, 73, 0 },
    { -375, -7, 0, 0 },
    { -367, -7, -73, 0 },
    { -346, -7, -143, 0 },
    { -311, -7, -208, 0 },
    { -265, -7, -265, 0 },
    { -208, -7, -311, 0 },
    { -143, -7, -346, 0 },
    { -73, -7, -367, 0 },
    { 0, -7, -375, 0 },
    { 73, -7, -367, 0 },
    { 143, -7, -346, 0 },
    { 208, -7, -311, 0 },
    { 265, -7, -265, 0 },
    { 311, -7, -208, 0 },
    { 346, -7, -143, 0 },
    { 367, -7, -73, 0 },
};
SVECTOR gRingDipPoints[64] = {
    { 375, -2, 0, 0 },
    { 367, -25, 73, 0 },
    { 346, -52, 143, 0 },
    { 311, -76, 208, 0 },
    { 265, -85, 265, 0 },
    { 208, -76, 311, 0 },
    { 143, -52, 346, 0 },
    { 73, -25, 367, 0 },
    { 0, -2, 375, 0 },
    { -73, 7, 367, 0 },
    { -143, 7, 346, 0 },
    { -208, 7, 311, 0 },
    { -265, 7, 265, 0 },
    { -311, 7, 208, 0 },
    { -346, 7, 143, 0 },
    { -367, 7, 73, 0 },
    { -375, 7, 0, 0 },
    { -367, 7, -73, 0 },
    { -346, 7, -143, 0 },
    { -311, 7, -208, 0 },
    { -265, 7, -265, 0 },
    { -208, 7, -311, 0 },
    { -143, 7, -346, 0 },
    { -73, 7, -367, 0 },
    { 0, 7, -375, 0 },
    { 73, 7, -367, 0 },
    { 143, 7, -346, 0 },
    { 208, 7, -311, 0 },
    { 265, 7, -265, 0 },
    { 311, 7, -208, 0 },
    { 346, 7, -143, 0 },
    { 367, 7, -73, 0 },
    { 375, -18, 0, 0 },
    { 367, -41, 73, 0 },
    { 346, -68, 143, 0 },
    { 311, -92, 208, 0 },
    { 265, -101, 265, 0 },
    { 208, -92, 311, 0 },
    { 143, -68, 346, 0 },
    { 73, -41, 367, 0 },
    { 0, -18, 375, 0 },
    { -73, -8, 367, 0 },
    { -143, -7, 346, 0 },
    { -208, -7, 311, 0 },
    { -265, -7, 265, 0 },
    { -311, -7, 208, 0 },
    { -346, -7, 143, 0 },
    { -367, -7, 73, 0 },
    { -375, -7, 0, 0 },
    { -367, -7, -73, 0 },
    { -346, -7, -143, 0 },
    { -311, -7, -208, 0 },
    { -265, -7, -265, 0 },
    { -208, -7, -311, 0 },
    { -143, -7, -346, 0 },
    { -73, -7, -367, 0 },
    { 0, -7, -375, 0 },
    { 73, -7, -367, 0 },
    { 143, -7, -346, 0 },
    { 208, -7, -311, 0 },
    { 265, -7, -265, 0 },
    { 311, -7, -208, 0 },
    { 346, -7, -143, 0 },
    { 367, -8, -73, 0 },
};
SVECTOR gRingBentDownPoints[64] = {
    { 375, 7, 0, 0 },
    { 367, 7, 73, 0 },
    { 346, 5, 143, 0 },
    { 311, 0, 208, 0 },
    { 265, -11, 265, 0 },
    { 208, -28, 311, 0 },
    { 143, -48, 346, 0 },
    { 73, -65, 367, 0 },
    { 0, -71, 375, 0 },
    { -73, -65, 367, 0 },
    { -143, -48, 346, 0 },
    { -208, -28, 311, 0 },
    { -265, -11, 265, 0 },
    { -311, 0, 208, 0 },
    { -346, 5, 143, 0 },
    { -367, 7, 73, 0 },
    { -375, 7, 0, 0 },
    { -367, 7, -73, 0 },
    { -346, 5, -143, 0 },
    { -311, 0, -208, 0 },
    { -265, -11, -265, 0 },
    { -208, -28, -311, 0 },
    { -143, -48, -346, 0 },
    { -73, -65, -367, 0 },
    { 0, -71, -375, 0 },
    { 73, -65, -367, 0 },
    { 143, -48, -346, 0 },
    { 208, -28, -311, 0 },
    { 265, -11, -265, 0 },
    { 311, 0, -208, 0 },
    { 346, 5, -143, 0 },
    { 367, 7, -73, 0 },
    { 375, -8, 0, 0 },
    { 367, -8, 73, 0 },
    { 346, -10, 143, 0 },
    { 311, -15, 208, 0 },
    { 265, -27, 265, 0 },
    { 208, -44, 311, 0 },
    { 143, -64, 346, 0 },
    { 73, -81, 367, 0 },
    { 0, -87, 375, 0 },
    { -73, -81, 367, 0 },
    { -143, -64, 346, 0 },
    { -208, -44, 311, 0 },
    { -265, -27, 265, 0 },
    { -311, -15, 208, 0 },
    { -346, -10, 143, 0 },
    { -367, -8, 73, 0 },
    { -375, -8, 0, 0 },
    { -367, -8, -73, 0 },
    { -346, -10, -143, 0 },
    { -311, -15, -208, 0 },
    { -265, -27, -265, 0 },
    { -208, -44, -311, 0 },
    { -143, -64, -346, 0 },
    { -73, -81, -367, 0 },
    { 0, -87, -375, 0 },
    { 73, -81, -367, 0 },
    { 143, -64, -346, 0 },
    { 208, -44, -311, 0 },
    { 265, -27, -265, 0 },
    { 311, -15, -208, 0 },
    { 346, -10, -143, 0 },
    { 367, -8, -73, 0 },
};
SVECTOR gRingBentUpPoints[64] = {
    { 375, 8, 0, 0 },
    { 367, 8, 73, 0 },
    { 346, 10, 143, 0 },
    { 311, 15, 208, 0 },
    { 265, 27, 265, 0 },
    { 208, 44, 311, 0 },
    { 143, 64, 346, 0 },
    { 73, 81, 367, 0 },
    { 0, 87, 375, 0 },
    { -73, 81, 367, 0 },
    { -143, 64, 346, 0 },
    { -208, 44, 311, 0 },
    { -265, 27, 265, 0 },
    { -311, 15, 208, 0 },
    { -346, 10, 143, 0 },
    { -367, 8, 73, 0 },
    { -375, 8, 0, 0 },
    { -367, 8, -73, 0 },
    { -346, 10, -143, 0 },
    { -311, 15, -208, 0 },
    { -265, 27, -265, 0 },
    { -208, 44, -311, 0 },
    { -143, 64, -346, 0 },
    { -73, 81, -367, 0 },
    { 0, 87, -375, 0 },
    { 73, 81, -367, 0 },
    { 143, 64, -346, 0 },
    { 208, 44, -311, 0 },
    { 265, 27, -265, 0 },
    { 311, 15, -208, 0 },
    { 346, 10, -143, 0 },
    { 367, 8, -73, 0 },
    { 375, -7, 0, 0 },
    { 367, -7, 73, 0 },
    { 346, -5, 143, 0 },
    { 311, 0, 208, 0 },
    { 265, 11, 265, 0 },
    { 208, 28, 311, 0 },
    { 143, 48, 346, 0 },
    { 73, 65, 367, 0 },
    { 0, 71, 375, 0 },
    { -73, 65, 367, 0 },
    { -143, 48, 346, 0 },
    { -208, 28, 311, 0 },
    { -265, 11, 265, 0 },
    { -311, 0, 208, 0 },
    { -346, -5, 143, 0 },
    { -367, -7, 73, 0 },
    { -375, -7, 0, 0 },
    { -367, -7, -73, 0 },
    { -346, -5, -143, 0 },
    { -311, 0, -208, 0 },
    { -265, 11, -265, 0 },
    { -208, 28, -311, 0 },
    { -143, 48, -346, 0 },
    { -73, 65, -367, 0 },
    { 0, 71, -375, 0 },
    { 73, 65, -367, 0 },
    { 143, 48, -346, 0 },
    { 208, 28, -311, 0 },
    { 265, 11, -265, 0 },
    { 311, 0, -208, 0 },
    { 346, -5, -143, 0 },
    { 367, -7, -73, 0 },
};
SVECTOR gLargeSpherePoints[80] = {
    { 638, 629, 0, 0 },
    { 590, 629, 244, 0 },
    { 451, 629, 451, 0 },
    { 244, 629, 590, 0 },
    { 0, 629, 638, 0 },
    { -244, 629, 590, 0 },
    { -451, 629, 451, 0 },
    { -590, 629, 244, 0 },
    { -638, 629, 0, 0 },
    { -590, 629, -244, 0 },
    { -451, 629, -451, 0 },
    { -244, 629, -590, 0 },
    { 0, 629, -638, 0 },
    { 244, 629, -590, 0 },
    { 451, 629, -451, 0 },
    { 590, 629, -244, 0 },
    { 836, 343, 0, 0 },
    { 773, 343, 320, 0 },
    { 591, 343, 591, 0 },
    { 320, 343, 773, 0 },
    { 0, 343, 836, 0 },
    { -320, 343, 773, 0 },
    { -591, 343, 591, 0 },
    { -773, 343, 320, 0 },
    { -836, 343, 0, 0 },
    { -773, 343, -320, 0 },
    { -591, 343, -591, 0 },
    { -320, 343, -773, 0 },
    { 0, 343, -836, 0 },
    { 320, 343, -773, 0 },
    { 591, 343, -591, 0 },
    { 773, 343, -320, 0 },
    { 899, 0, 0, 0 },
    { 831, 0, 344, 0 },
    { 636, 0, 636, 0 },
    { 344, 0, 831, 0 },
    { 0, 0, 899, 0 },
    { -344, 0, 831, 0 },
    { -636, 0, 636, 0 },
    { -831, 0, 344, 0 },
    { -899, 0, 0, 0 },
    { -831, 0, -344, 0 },
    { -636, 0, -636, 0 },
    { -344, 0, -831, 0 },
    { 0, 0, -899, 0 },
    { 344, 0, -831, 0 },
    { 636, 0, -636, 0 },
    { 831, 0, -344, 0 },
    { 836, -343, 0, 0 },
    { 773, -343, 320, 0 },
    { 591, -343, 591, 0 },
    { 320, -343, 773, 0 },
    { 0, -343, 836, 0 },
    { -320, -343, 773, 0 },
    { -591, -343, 591, 0 },
    { -773, -343, 320, 0 },
    { -836, -343, 0, 0 },
    { -773, -343, -320, 0 },
    { -591, -343, -591, 0 },
    { -320, -343, -773, 0 },
    { 0, -343, -836, 0 },
    { 320, -343, -773, 0 },
    { 591, -343, -591, 0 },
    { 773, -343, -320, 0 },
    { 638, -629, 0, 0 },
    { 590, -629, 244, 0 },
    { 451, -629, 451, 0 },
    { 244, -629, 590, 0 },
    { 0, -629, 638, 0 },
    { -244, -629, 590, 0 },
    { -451, -629, 451, 0 },
    { -590, -629, 244, 0 },
    { -638, -629, 0, 0 },
    { -590, -629, -244, 0 },
    { -451, -629, -451, 0 },
    { -244, -629, -590, 0 },
    { 0, -629, -638, 0 },
    { 244, -629, -590, 0 },
    { 451, -629, -451, 0 },
    { 590, -629, -244, 0 },
};
SVECTOR gSmallSpherePoints[84] = {
    { 61, -147, 0, 0 },
    { 53, -147, 30, 0 },
    { 30, -147, 53, 0 },
    { 0, -147, 61, 0 },
    { -30, -147, 53, 0 },
    { -53, -147, 30, 0 },
    { -61, -147, 0, 0 },
    { -53, -147, -30, 0 },
    { -30, -147, -53, 0 },
    { 0, -147, -61, 0 },
    { 30, -147, -53, 0 },
    { 53, -147, -30, 0 },
    { 113, -113, 0, 0 },
    { 97, -113, 56, 0 },
    { 56, -113, 97, 0 },
    { 0, -113, 113, 0 },
    { -56, -113, 97, 0 },
    { -97, -113, 56, 0 },
    { -113, -113, 0, 0 },
    { -97, -113, -56, 0 },
    { -56, -113, -97, 0 },
    { 0, -113, -113, 0 },
    { 56, -113, -97, 0 },
    { 97, -113, -56, 0 },
    { 147, -61, 0, 0 },
    { 128, -61, 73, 0 },
    { 73, -61, 128, 0 },
    { 0, -61, 147, 0 },
    { -73, -61, 128, 0 },
    { -128, -61, 73, 0 },
    { -147, -61, 0, 0 },
    { -128, -61, -73, 0 },
    { -73, -61, -128, 0 },
    { 0, -61, -147, 0 },
    { 73, -61, -128, 0 },
    { 128, -61, -73, 0 },
    { 160, 0, 0, 0 },
    { 138, 0, 80, 0 },
    { 80, 0, 138, 0 },
    { 0, 0, 160, 0 },
    { -80, 0, 138, 0 },
    { -138, 0, 80, 0 },
    { -160, 0, 0, 0 },
    { -138, 0, -80, 0 },
    { -80, 0, -138, 0 },
    { 0, 0, -160, 0 },
    { 80, 0, -138, 0 },
    { 138, 0, -80, 0 },
    { 147, 61, 0, 0 },
    { 128, 61, 73, 0 },
    { 73, 61, 128, 0 },
    { 0, 61, 147, 0 },
    { -73, 61, 128, 0 },
    { -128, 61, 73, 0 },
    { -147, 61, 0, 0 },
    { -128, 61, -73, 0 },
    { -73, 61, -128, 0 },
    { 0, 61, -147, 0 },
    { 73, 61, -128, 0 },
    { 128, 61, -73, 0 },
    { 113, 113, 0, 0 },
    { 97, 113, 56, 0 },
    { 56, 113, 97, 0 },
    { 0, 113, 113, 0 },
    { -56, 113, 97, 0 },
    { -97, 113, 56, 0 },
    { -113, 113, 0, 0 },
    { -97, 113, -56, 0 },
    { -56, 113, -97, 0 },
    { 0, 113, -113, 0 },
    { 56, 113, -97, 0 },
    { 97, 113, -56, 0 },
    { 61, 147, 0, 0 },
    { 53, 147, 30, 0 },
    { 30, 147, 53, 0 },
    { 0, 147, 61, 0 },
    { -30, 147, 53, 0 },
    { -53, 147, 30, 0 },
    { -61, 147, 0, 0 },
    { -53, 147, -30, 0 },
    { -30, 147, -53, 0 },
    { 0, 147, -61, 0 },
    { 30, 147, -53, 0 },
    { 53, 147, -30, 0 },
};
VECTOR gQuadOrbitPoint = { 200, 0, 150, 0 };
u_char gStreakColors[4][3] = {
    { 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0x80 },
    { 0x40, 0x40, 0x40 },
    { 0x00, 0x80, 0x80 },
};
