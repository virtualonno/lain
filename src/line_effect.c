/* The start screen's sixteen fading lines: each trails a point that loops out from its
   origin along a random curve and back. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include "graphics.h"
#include "heap.h"
#include "line_effect.h"

#define TRAIL_COUNT 16
#define TRAIL_HISTORY 256
#define TRAIL_DRAWN 128

typedef struct {
    int x;
    int y;
} TrailPoint;

TrailPoint *gTrailPoints;
u_short gTrailPointCount;

static TrailPoint *sTrailHistory[TRAIL_COUNT];
/* A loop's point is origin + t*t*(1-t)*[0] + t*(1-t)*(1-t)*[1], t from 0 to 1. */
static TrailPoint sTrailControl[TRAIL_COUNT][2];
static u_short sTrailPhase[TRAIL_COUNT]; /* t in 256ths */
static u_short sTrailMirror[TRAIL_COUNT];
static u_short sTrailHead[TRAIL_COUNT];
static u_short sTrailLength[TRAIL_COUNT];
static u_short sTrailOrigin[TRAIL_COUNT][2];
static GsGLINE *sTrailLines[TRAIL_COUNT];

void DrawLineTrail(int index)
{
    volatile int i;
    short b0;
    short b1;

    for (i = 0; i < gTrailPointCount - 1; i++) {
        b0 = (128 - i) * 100 / 128;
        b1 = (127 - i) * 100 / 128;
        sTrailLines[index][i].attribute = 0;
        sTrailLines[index][i].x0 = gTrailPoints[i + 1].x;
        sTrailLines[index][i].y0 = gTrailPoints[i + 1].y;
        sTrailLines[index][i].x1 = gTrailPoints[i].x;
        sTrailLines[index][i].y1 = gTrailPoints[i].y;
        sTrailLines[index][i].r0 = 0;
        sTrailLines[index][i].g0 = 0;
        sTrailLines[index][i].b0 = b0;
        sTrailLines[index][i].r1 = 0;
        sTrailLines[index][i].g1 = 0;
        sTrailLines[index][i].b1 = b1;
        GsSortGLine(&sTrailLines[index][i], &gSubOt[gActiveBuffer], 4);
    }
}

void SeedTrailCurve(int index, int mirror)
{
    sTrailControl[index][0].x = rand() % 480;
    sTrailControl[index][0].y = rand() % 360;
    sTrailControl[index][1].x = rand() % 480;
    sTrailControl[index][1].y = -(rand() % 360);
    if (mirror) {
        sTrailControl[index][0].x = -sTrailControl[index][0].x;
    }
}

void AdvanceTrailPoint(int index)
{
    int t;
    int u;
    int weight0;
    int weight1;
    TrailPoint *control;
    int x;
    int y;

    t = sTrailPhase[index];
    weight0 = t * t;
    u = 256 - t;
    weight1 = u * u;
    weight0 >>= 8;
    weight1 >>= 8;
    weight0 *= u;
    weight1 *= t;
    weight0 >>= 8;
    weight1 >>= 8;
    control = sTrailControl[index];
    x = weight0 * control[0].x + weight1 * control[1].x;
    y = weight0 * control[0].y + weight1 * control[1].y;
    x = (x >> 8) + sTrailOrigin[index][0];
    y = (y >> 8) + sTrailOrigin[index][1];
    sTrailHistory[index][sTrailHead[index]].x = x;
    sTrailHistory[index][sTrailHead[index]].y = y;
    sTrailHead[index] = (sTrailHead[index] + 1) & (TRAIL_HISTORY - 1);
    if (sTrailLength[index] < 255) {
        sTrailLength[index]++;
    }
}

void InitLineTrails(void)
{
    int i;
    int phase;

    gTrailPoints = (TrailPoint *)AllocHeapBlock(TRAIL_HISTORY * sizeof(TrailPoint));
    phase = 0;
    for (i = 0; i < TRAIL_COUNT; i++) {
        sTrailLines[i] = (GsGLINE *)AllocHeapBlock(TRAIL_DRAWN * sizeof(GsGLINE));
        sTrailHistory[i] = (TrailPoint *)AllocHeapBlock(TRAIL_HISTORY * sizeof(TrailPoint));
        sTrailHead[i] = 0;
        sTrailLength[i] = 0;
        sTrailMirror[i] = i & 1;
        sTrailOrigin[i][0] = rand() % 20 + 150;
        sTrailOrigin[i][1] = rand() % 20 + 90;
        sTrailPhase[i] = phase & 0xff;
        SeedTrailCurve(i, sTrailMirror[i]);
        phase += 16;
    }
}

void StepLineTrails(void)
{
    char unusedFrame[88]; /* kept: 88 bytes of frame room nothing reads; without it the frame is 48 bytes, not 136 */
    int index;
    int step;
    int pointCount;
    int ringIndex;
    TrailPoint *history;
    int copied;
    int x;

    for (step = 0; step < 4; step++) {
        for (index = 0; index < TRAIL_COUNT; index++) {
            if (sTrailPhase[index] == 0) {
                if (sTrailMirror[index] != 0) {
                    sTrailMirror[index] = 0;
                } else {
                    sTrailMirror[index] = 1;
                }
                SeedTrailCurve(index, sTrailMirror[index]);
            }
            AdvanceTrailPoint(index);
            if (++sTrailPhase[index] >= 256) {
                sTrailPhase[index] = 0;
            }
        }
    }
    for (index = 0; index < TRAIL_COUNT; index++) {
        pointCount = sTrailLength[index] < TRAIL_DRAWN ? sTrailLength[index] : TRAIL_DRAWN;
        ringIndex = sTrailHead[index];
        gTrailPointCount = 0;
        for (copied = 0; copied < pointCount; copied++) {
            ringIndex = ringIndex - 1 < 0 ? TRAIL_HISTORY - 1 : ringIndex - 1;
            history = sTrailHistory[index];
            x = history[ringIndex].x;
            gTrailPoints[gTrailPointCount].x = x;
            gTrailPoints[gTrailPointCount].y = history[ringIndex].y;
            gTrailPointCount++;
        }
        DrawLineTrail(index);
    }
}

void FreeLineTrails(void)
{
    int i;

    for (i = 0; i < TRAIL_COUNT; i++) {
        FreeHeapBlock(sTrailLines[i]);
        FreeHeapBlock(sTrailHistory[i]);
    }
    FreeHeapBlock(gTrailPoints);
}
