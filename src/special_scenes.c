/* Auxiliary 3D scenes: the post-end-roll collapsing spheres (gModels[40..46], run for track 733)
 * and the kind-8 gate scene (gModels[40..48] carousel, Bezier piece flight, and TIM column wrap).
 * Joined in one unit by .sdata layout. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>

#include "about.h"
#include "special_scenes.h"
#include "graphics.h"
#include "grid_menu.h"
#include "grid_scene.h"
#include "memcard.h"
#include "grid_effects.h"
#include "tmd_data_5.h"

int gWaitFrames; /* kept: this unit reads gWaitFrames as an int; src/grid_scene.c declares it as unsigned short */


extern u_long gPointTmd[2];

extern u_long gGateSlabTim;
extern u_long gGateBackdropTim;
extern u_long gGateBackdropTimFlags;
SVECTOR gSphereCollapseOrigin = { 0, 0, 0, 0 };
short gGateCenterX = 0;
short gGateCenterY = 0;
short gGateCenterZ = 100;
short gUnusedGateShort = 0; /* 2 bytes; nothing reads it */
int gUnusedGateWord = 0; /* 4 bytes; nothing reads it */
short gGateFlightStartZ = -300;
u_char gGateTimRotated;
u_char gGateFlightReady;

enum SphereCollapseStep {
    SPHERE_COLLAPSE_INIT_SMALL,
    SPHERE_COLLAPSE_SMALL,
    SPHERE_COLLAPSE_INIT_LARGE,
    SPHERE_COLLAPSE_LARGE,
    SPHERE_COLLAPSE_FINISH,
    SPHERE_COLLAPSE_DONE = 0xff
};
u_char gSphereCollapseStep;
u_char gSphereCollapseDone;
static SVECTOR sSphereCollapseVertices[84];
static SVECTOR sSphereCollapseTargets[84];
static GsSPRITE sGateBackdropSprites[30];

void InitEndRollSpheres(void)
{
    RECT rect;
    struct TMD_STRUCT info;
    Model *parent, *model;
    unsigned int i;
    int child;

    LoadTimPart(30);
    rect.w = 320;
    rect.x = 0;
    rect.y = 0;
    rect.h = 480;
    ClearImage2(&rect, 0, 0, 0);
    LoadTimPart(26);
    LoadTimPart(27);
    LoadTimPart(28);
    LoadTimPart(29);

    parent = &gModels[40];
    InitModelCoordinate(parent);
    SetModelTmd(gPointTmd, parent);
    LinkModelObjects(parent, 0);
    gModels[40].coord.coord.t[0] = 0;
    gModels[40].coord.coord.t[1] = 0;
    gModels[40].coord.coord.t[2] = 0;
    model = parent;
    UpdateModelMatrix(model++);
    InitModelCoordinate(model);
    SetModelTmd(gLargeSphereTmd, model);
    LinkModelObjects(model, &parent->coord);
    info = *gModels[41].objects;
    for (child = 0; child < info.vern; child++) {
        SetTmdVertex(child, &gLargeSpherePoints[child], &info);
    }
    gModels[41].coord.coord.t[0] = 0;
    gModels[41].coord.coord.t[1] = 0;
    gModels[41].coord.coord.t[2] = 0;
    gModels[41].object.attribute = 0;
    UpdateModelMatrix(&gModels[41]);

    for (child = 1; child < 6; child++) {
        model = &gModels[41 + child];
        InitModelCoordinate(model);
        SetModelTmd(gSmallSphereTmd, &gModels[41 + child]);
        LinkModelObjects(&gModels[41 + child], &gModels[40].coord);
        info = *gModels[41 + child].objects;
        for (i = 0; i < info.vern; ++i) {
            SetTmdVertex(i, &gSmallSpherePoints[i], &info);
        }
        gModels[41 + child].coord.coord.t[0] = gSmallSpherePositions[child - 1].vx;
        gModels[41 + child].coord.coord.t[1] = gSmallSpherePositions[child - 1].vy;
        gModels[41 + child].coord.coord.t[2] = gSmallSpherePositions[child - 1].vz;
        gModels[41 + child].object.attribute = 0;
        UpdateModelMatrix(&gModels[41 + child]);
    }
    gWaitFrames = 0;
    gSphereCollapseStep = SPHERE_COLLAPSE_INIT_SMALL;
    gSphereCollapseDone = 0;
    SetViewLights();
    gFlatLights[2].vz = -700;
    gFlatLights[2].vx = 0;
    gFlatLights[2].vy = 0;
    gFlatLights[2].g = 0x30;
    gFlatLights[2].r = 0x30;
    GsSetFlatLight(2, &gFlatLights[2]);
    DrawSync(0);
}


void StepAndSortEndRollSpheres(int stepDelay)
{
    int modelIndex;

    if (gWaitFrames >= stepDelay) {
        gModels[41].rot.vy += 22;
        UpdateModelMatrix(&gModels[41]);
        for (modelIndex = 1; modelIndex < 6; modelIndex++) {
            gModels[41 + modelIndex].rot.vy -= 22;
            UpdateModelMatrix(&gModels[41 + modelIndex]);
        }
        gWaitFrames = 0;
    } else {
        gWaitFrames++;
    }
    SortModel(&gModels[41], gOt[gActiveBuffer]);
    for (modelIndex = 1; modelIndex < 6; modelIndex++) {
        SortModel(&gModels[41 + modelIndex], gOt[gActiveBuffer]);
    }
}

void StepEndRollSpheresCollapse(void)
{
    struct TMD_STRUCT info;
    int i;

    switch (gSphereCollapseStep) {
    case SPHERE_COLLAPSE_INIT_SMALL:
        gSavedPointDeltas[0] = gPointDeltas[0];
        gSavedPointDeltaStepsLeft = gPointDeltaStepsLeft;
        gSavedPointDeltaMode = gPointDeltaMode;
        for (i = 0; i < 84; i++) {
            sSphereCollapseTargets[i].vx = gSphereCollapseOrigin.vx;
            sSphereCollapseTargets[i].vy = gSphereCollapseOrigin.vy;
            sSphereCollapseTargets[i].vz = gSphereCollapseOrigin.vz;
        }
        info = *gModels[42].objects;
        for (i = 0; i < info.vern; i++) {
            GetTmdVertex(i, &sSphereCollapseVertices[i], &info);
        }
        gPointDeltaMode = POINT_DELTA_SPHERE_COLLAPSE;
        InitPointDeltas(sSphereCollapseTargets, sSphereCollapseVertices, 30, 84);
        gSphereCollapseStep = SPHERE_COLLAPSE_SMALL;
        break;
    case SPHERE_COLLAPSE_SMALL:
        if (gPointDeltaStepsLeft > 0 && gPointDeltaMode == POINT_DELTA_SPHERE_COLLAPSE) {
            ApplyPointDeltas(sSphereCollapseVertices);
            info = *gModels[42].objects;
            for (i = 0; i < info.vern; i++) {
                SetTmdVertex(i, &sSphereCollapseVertices[i], &info);
            }
        } else {
            for (i = 1; i < 6; i++) {
                gModels[41 + i].object.attribute |= GsDOFF;
            }
            gPointDeltas[0] = gSavedPointDeltas[0];
            gSphereCollapseStep = SPHERE_COLLAPSE_INIT_LARGE;
            gPointDeltaStepsLeft = gSavedPointDeltaStepsLeft;
            gPointDeltaMode = gSavedPointDeltaMode;
        }
        break;
    case SPHERE_COLLAPSE_INIT_LARGE:
        gSavedPointDeltas[0] = gPointDeltas[0];
        gSavedPointDeltaStepsLeft = gPointDeltaStepsLeft;
        gSavedPointDeltaMode = gPointDeltaMode;
        for (i = 0; i < 80; i++) {
            sSphereCollapseTargets[i].vx = gSphereCollapseOrigin.vx;
            sSphereCollapseTargets[i].vy = gSphereCollapseOrigin.vy;
            sSphereCollapseTargets[i].vz = gSphereCollapseOrigin.vz;
        }
        info = *gModels[41].objects;
        for (i = 0; i < info.vern; i++) {
            GetTmdVertex(i, &sSphereCollapseVertices[i], &info);
        }
        gPointDeltaMode = POINT_DELTA_SPHERE_COLLAPSE;
        InitPointDeltas(sSphereCollapseTargets, sSphereCollapseVertices, 60, 80);
        gSphereCollapseStep = SPHERE_COLLAPSE_LARGE;
        break;
    case SPHERE_COLLAPSE_LARGE:
        if (gPointDeltaStepsLeft > 0 && gPointDeltaMode == POINT_DELTA_SPHERE_COLLAPSE) {
            ApplyPointDeltas(sSphereCollapseVertices);
            info = *gModels[41].objects;
            for (i = 0; i < info.vern; i++) {
                SetTmdVertex(i, &sSphereCollapseVertices[i], &info);
            }
        } else {
            gPointDeltas[0] = gSavedPointDeltas[0];
            gSphereCollapseStep = SPHERE_COLLAPSE_FINISH;
            gModels[41].object.attribute |= GsDOFF;
            gPointDeltaStepsLeft = gSavedPointDeltaStepsLeft;
            gPointDeltaMode = gSavedPointDeltaMode;
        }
        break;
    case SPHERE_COLLAPSE_FINISH:
        gSphereCollapseDone = 0xff;
        gSphereCollapseStep = SPHERE_COLLAPSE_DONE;
        break;
    }
    if (gSphereCollapseDone == 0) {
        StepAndSortEndRollSpheres(0);
    }
}

void InitGateScene(void)
{
    u_int i;
    DVECTOR pos;
    SVECTOR unused; /* kept: nothing reads it; without it the routine compiles differently */
    GsIMAGE image;

    InitModelCoordinate(&gModels[40]);
    SetModelTmd(gPointTmd, &gModels[40]);
    LinkModelObjects(&gModels[40], 0);
    gModels[40].coord.coord.t[0] = gGateCenterX;
    gModels[40].coord.coord.t[1] = gGateCenterY;
    gModels[40].coord.coord.t[2] = gGateCenterZ;
    UpdateModelMatrix(&gModels[40]);
    for (i = 0; i < 4; i++) {
        InitModelCoordinate(&gModels[i + 41]);
        SetModelTmd(gGateSlabTmds[i], &gModels[i + 41]);
        LinkModelObjects(&gModels[i + 41], &gModels[40].coord);
        gModels[i + 41].coord.coord.t[0] = gGateModelPositions[i].vx;
        gModels[i + 41].coord.coord.t[1] = gGateModelPositions[i].vy;
        gModels[i + 41].coord.coord.t[2] = gGateModelPositions[i].vz;
        gModels[i + 41].rot.vy = i << 10;
        gModels[i + 41].object.attribute = GsDOFF;
        UpdateModelMatrix(&gModels[i + 41]);
    }
    InitModelCoordinate(&gModels[46]);
    SetModelTmd(gGateSlabTmds[4], &gModels[46]);
    LinkModelObjects(&gModels[46], 0);
    gModels[46].coord.coord.t[0] = (rand() & 0x1ff) - 160;
    gModels[46].coord.coord.t[1] = (rand() & 0xff) - 120;
    gModels[46].coord.coord.t[2] = gGateFlightStartZ;
    gModels[46].object.attribute = GsDOFF;
    UpdateModelMatrix(&gModels[46]);
    ProjectModelToScreen(&gModels[46], &pos);
    gGateFlightStart.vx = pos.vx;
    gGateFlightStart.vy = pos.vy;
    InitModelCoordinate(&gModels[47]);
    SetModelTmd(gGateLeftPlaneTmd, &gModels[47]);
    LinkModelObjects(&gModels[47], 0);
    gModels[47].coord.coord.t[0] = 0;
    gModels[47].coord.coord.t[1] = 0;
    gModels[47].coord.coord.t[2] = 0;
    gModels[47].object.attribute = 0;
    UpdateModelMatrix(&gModels[47]);
    InitModelCoordinate(&gModels[48]);
    SetModelTmd(gGateRightPlaneTmd, &gModels[48]);
    LinkModelObjects(&gModels[48], 0);
    gModels[48].coord.coord.t[0] = 0;
    gModels[48].coord.coord.t[1] = 0;
    gModels[48].coord.coord.t[2] = 0;
    gModels[48].object.attribute = 0;
    UpdateModelMatrix(&gModels[48]);
    gFlatLights[2].vx = 0;
    gFlatLights[2].vy = 0;
    gFlatLights[2].vz = -700;
    gFlatLights[2].g = 0x30;
    gFlatLights[2].r = 0x30;
    GsSetFlatLight(2, &gFlatLights[2]);
    gGateFlightReady = 0;
    gGateTimRotated = 0;
    UploadTim(&gGateSlabTim);
    UploadTim(&gGateBackdropTim);
    GsGetTimInfo(&gGateBackdropTimFlags, &image);
    for (i = 0; i < 30; i++) {
        sGateBackdropSprites[i].attribute = 0x60000000;
        sGateBackdropSprites[i].x = 160 - image.pw * 2;
        sGateBackdropSprites[i].y = i * image.ph + image.ph / 2;
        sGateBackdropSprites[i].r = sGateBackdropSprites[i].g = sGateBackdropSprites[i].b = 128;
        sGateBackdropSprites[i].scalex = sGateBackdropSprites[i].scaley = 1;
        sGateBackdropSprites[i].rotate = 0;
        sGateBackdropSprites[i].tpage = GetTPage(image.pmode, 2, image.px, image.py);
        sGateBackdropSprites[i].u = 0;
        sGateBackdropSprites[i].v = image.py;
        sGateBackdropSprites[i].w = image.pw * 4;
        sGateBackdropSprites[i].h = image.ph;
        sGateBackdropSprites[i].cx = image.cx;
        sGateBackdropSprites[i].cy = image.cy;
    }
}

int gGateAlignYaws[4] = { 0, 12, 8, 4 };
double gGateFlightStep;

void UpdateGateScene(void)
{
    MATRIX lw;
    MATRIX ls;
    MATRIX m;
    double pos[3];
    SVECTOR v;
    SVECTOR ang;
    SVECTOR unused; /* kept: nothing reads it; without it the routine compiles differently */
    VECTOR out;
    VECTOR unused2; /* kept: nothing reads it; without it the routine compiles differently */
    u_int i;
    int n;

    for (i = 0; i < 30; i++) {
        GsSortFastSprite(&sGateBackdropSprites[i], &gOt[gActiveBuffer], 700);
    }
    if (gGatesOpened != 0) {
        gModels[40].rot.vy = (gModels[40].rot.vy + 22) & 0xfff;
        UpdateModelMatrix(&gModels[40]);
    }
    if (gGatesOpened < 5) {
        n = gGatesOpened;
    } else {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        gModels[41 + i].object.attribute &= ~GsDOFF;
        SortModel(&gModels[41 + i], gOt[gActiveBuffer]);
    }
    if (gGatesOpened < 4) {
        if (gGateFlightReady == 0) {
            gGateFlightReady = 0xff;
            gGateFlightStep = 0;
            gModels[46].object.attribute &= ~GsDOFF;
            GsGetLws(&gModels[46].coord, &lw, &ls);
            PushMatrix();
            ang.vy = 0;
            ang.vx = 0;
            ang.vz = 0;
            RotMatrix(&ang, &m);
            SetRotMatrix(&m);
            v.vx = lw.t[0];
            v.vy = lw.t[1];
            v.vz = lw.t[2];
            ApplyRotMatrix(&v, &out);
            gBezierPoints[0][0] = out.vx;
            gBezierPoints[0][1] = out.vy;
            gBezierPoints[0][2] = out.vz;
            v.vx = 0;
            v.vy = 0;
            v.vz = 0;
            ApplyRotMatrix(&v, &out);
            gBezierPoints[1][0] = out.vx;
            gBezierPoints[1][1] = out.vy;
            gBezierPoints[1][2] = out.vz;
            v.vx = gGateCenterX;
            v.vy = gGateCenterY;
            v.vz = gGateCenterZ;
            ApplyRotMatrix(&v, &out);
            gBezierPoints[2][0] = out.vx;
            gBezierPoints[2][1] = out.vy;
            gBezierPoints[2][2] = out.vz;
            PopMatrix();
            WeightBezierPoints(3, gBezierPoints, gWeightedBezierPoints);
            SortModel(&gModels[46], gOt[gActiveBuffer]);
        } else if (gGateFlightActive != 0) {
            if (gGateFlightStep < 60.0) {
                EvalBezier(3, gWeightedBezierPoints, pos, gGateFlightStep / 60.0);
                gModels[46].coord.coord.t[0] = pos[0];
                gModels[46].coord.coord.t[1] = pos[1];
                gModels[46].coord.coord.t[2] = pos[2];
                UpdateModelMatrix(&gModels[46]);
                gGateFlightStep += 1.0;
            }
            SortModel(&gModels[46], gOt[gActiveBuffer]);
            if (gGateFlightStep >= 60.0) {
                if (gModels[40].rot.vy >> 8 == gGateAlignYaws[gGatesOpened]) {
                    gModels[46].object.attribute |= GsDOFF;
                    gGateFlightReady = 0;
                    gGateFlightActive = 0;
                    gModels[41 + gGatesOpened].object.attribute &= ~GsDOFF;
                    SortModel(&gModels[41 + gGatesOpened], gOt[gActiveBuffer]);
                }
            }
        }
    }
    if (gModels[40].rot.vy & 0x20) {
        if (gGateTimRotated == 0) {
            RotateTimColumnsRight();
        }
    } else {
        gGateTimRotated = 0;
    }
    SortModel(&gModels[47], gOt[gActiveBuffer]);
    SortModel(&gModels[48], gOt[gActiveBuffer]);
}

extern u_long gGateSlabTimFlags;

void RotateTimColumnsRight(void)
{
    GsIMAGE image;
    RECT rect;

    GsGetTimInfo(&gGateSlabTimFlags, &image);
    rect.x = image.px;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    MoveImage(&rect, (short)image.px + image.pw, (short)image.py);
    DrawSync(0);

    rect.x = image.px + image.pw - 1;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    MoveImage(&rect, (short)image.px + image.pw, (short)image.py);
    DrawSync(0);

    rect.x = image.px + image.pw;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    MoveImage(&rect, (short)image.px, (short)image.py);
    DrawSync(0);
    gGateTimRotated++;
}
