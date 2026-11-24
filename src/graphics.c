/* 320x240 double-buffered display setup, two-tier ordering tables (gOt, gSubOt),
   view/light/fog/CLUT-fade helpers, TMD/TIM loading, Bezier paths, and the
   gModels[51], gCellModels[103], and gLevelModels[21] model pools. */

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>

#include "graphics.h"
#include "libgs_int.h"
#include "tmd_data_5.h"

int gActiveBuffer;

short gClutFading;
u_short gClut[256];
u_short gTargetClut[256];

int gFadeLightIndex;
u_char gLightFading;
GsF_LIGHT *gFadeLightTarget;

GsOT_TAG gOtTags0[16384];
GsOT_TAG gOtTags1[16384];
GsOT_TAG gSubOtTags[2][256];
int gLinkedObjectCount;

int InitGraphics(void)
{
    RECT screen;

    gLinkedObjectCount = 0;
    ResetGraph(0);
    DrawSync(0);
    InitGeom();
    GsInitGraph(320, 240, 4, 1, 0);
    GsDefDispBuff2(0, 0, 0, 240);
    GsInit3D();
    GsSetOrign(0, 0);
    SetGeomOffset(160, 120);
    gOt[0].length = 14;
    gOt[0].org = gOtTags0;
    gOt[1].length = 14;
    gOt[1].org = gOtTags1;
    gSubOt[0].length = 8;
    gSubOt[0].org = gSubOtTags[0];
    gSubOt[1].length = 8;
    gSubOt[1].org = gSubOtTags[1];
    InitView();
    SetViewLights();
    GsSetAmbient(128, 128, 128);
    GsSetLightMode(0);
    screen.w = 320;
    screen.x = 0;
    screen.y = 0;
    screen.h = 512;
    return ClearImage2(&screen, 0, 0, 0);
}

u_char gViewMode;

void SetViewLights(void)
{
    int i;
    for (i = 0; i < 3; i++) {
        switch (gViewMode) {
        case VIEW_TOP:
        case VIEW_DESCENT:
            gFlatLights[i] = gTopViewLights[i];
            break;
        case VIEW_FRONT_RETURN:
            gFlatLights[i] = gFrontViewLights[i];
            break;
        default:
            gFlatLights[i] = gFrontViewLights[i];
            break;
        }
        GsSetFlatLight(i, &gFlatLights[i]);
    }
}

void StepLightFade(void)
{
    if (gLightFading == 1) {
        if (gFlatLights[gFadeLightIndex].r != gFadeLightTarget->r) {
            if (gFlatLights[gFadeLightIndex].r > gFadeLightTarget->r) {
                gFlatLights[gFadeLightIndex].r -= 2;
            } else if (gFlatLights[gFadeLightIndex].r < gFadeLightTarget->r) {
                gFlatLights[gFadeLightIndex].r += 2;
            }
        }
        if (gFlatLights[gFadeLightIndex].g != gFadeLightTarget->g) {
            if (gFlatLights[gFadeLightIndex].g > gFadeLightTarget->g) {
                gFlatLights[gFadeLightIndex].g -= 2;
            } else if (gFlatLights[gFadeLightIndex].g < gFadeLightTarget->g) {
                gFlatLights[gFadeLightIndex].g += 2;
            }
        }
        if (gFlatLights[gFadeLightIndex].b != gFadeLightTarget->b) {
            if (gFlatLights[gFadeLightIndex].b > gFadeLightTarget->b) {
                gFlatLights[gFadeLightIndex].b -= 2;
            } else if (gFlatLights[gFadeLightIndex].b < gFadeLightTarget->b) {
                gFlatLights[gFadeLightIndex].b += 2;
            }
        }
        GsSetFlatLight(gFadeLightIndex, &gFlatLights[gFadeLightIndex]);
        if (gFlatLights[gFadeLightIndex].r == gFadeLightTarget->r && gFlatLights[gFadeLightIndex].g == gFadeLightTarget->g
            && gFlatLights[gFadeLightIndex].b == gFadeLightTarget->b) {
            gLightFading = 0;
        }
    }
}

void StepClutFade(u_long *image, u_long *target)
{
    GsIMAGE tim;
    RECT rect;
    RECT targetRect;
    u_char cur[3];
    u_char goal[3];
    int count;
    int i;

    gClutFading = 0;
    GsGetTimInfo(image + 1, &tim);
    rect.x = tim.cx;
    rect.y = tim.cy;
    rect.w = tim.cw;
    rect.h = tim.ch;
    StoreImage(&rect, (u_long *)gClut);
    DrawSync(0);
    if (tim.pmode & 8) {
        if ((tim.pmode & 7) == 0) {
            count = 16;
        } else {
            count = 256;
        }
        GsGetTimInfo(target + 1, &tim);
        targetRect.x = tim.cx;
        targetRect.y = tim.cy;
        targetRect.w = tim.cw;
        targetRect.h = tim.ch;
        StoreImage(&targetRect, (u_long *)gTargetClut);
        DrawSync(0);
        for (i = 0; i < count; i++) {
            cur[0] = gClut[i] & 0x1f;
            cur[1] = (gClut[i] >> 5) & 0x1f;
            cur[2] = (gClut[i] >> 10) & 0x1f;
            goal[0] = gTargetClut[i] & 0x1f;
            goal[1] = (gTargetClut[i] >> 5) & 0x1f;
            goal[2] = (gTargetClut[i] >> 10) & 0x1f;
            if (cur[0] > goal[0]) {
                cur[0]--;
            } else if (cur[0] < goal[0]) {
                cur[0]++;
            }
            if (cur[1] > goal[1]) {
                cur[1]--;
            } else if (cur[1] < goal[1]) {
                cur[1]++;
            }
            if (cur[2] > goal[2]) {
                cur[2]--;
            } else if (cur[2] < goal[2]) {
                cur[2]++;
            }
            gClut[i] = cur[0];
            gClut[i] |= cur[1] << 5;
            gClut[i] |= cur[2] << 10;
            gClut[i] |= gTargetClut[i] & 0x8000;
            if (gClut[i] != gTargetClut[i]) {
                gClutFading = 255;
            }
        }
        LoadImage(&rect, (u_long *)gClut);
        DrawSync(0);
    }
}

void StepClutToBlack(u_long *image)
{
    GsIMAGE tim;
    RECT rect;
    CVECTOR rgb;
    u_short *palette;
    u_short *entry;
    int mode, count, i;
    int r, g, b;
    int packed;

    gClutFading = 0;
    GsGetTimInfo(image + 1, &tim);
    palette = gClut;
    rect.x = tim.cx;
    rect.y = tim.cy;
    rect.w = tim.cw;
    rect.h = tim.ch;
    StoreImage(&rect, (u_long *)palette);
    DrawSync(0);

    mode = tim.pmode;
    if (mode & 8) {
        count = 256;
        if (!(mode & 7))
            count = 16;

        for (i = 0; i < count; ++i) {
            entry = &gClut[i];
            gTargetClut[0] = *entry & 0x8000;
            rgb.r = r = *(u_char *)entry & 31;
            rgb.g = g = (*entry >> 5) & 31;
            rgb.b = b = (*entry >> 10) & 31;

            if (r != 0)
                --rgb.r;
            if (g != 0)
                --rgb.g;
            if (b != 0)
                --rgb.b;

            *entry = packed = rgb.r;
            *entry = packed |= rgb.g << 5;
            *entry = packed |= rgb.b << 10;
            if (*entry) {
                gClutFading = 0xff;
            }
            *entry = *entry | gTargetClut[0];
        }

        LoadImage(&rect, (u_long *)gClut);
        DrawSync(0);
    }
}

Model gModels[51];
Model gCellModels[103];
Model gLevelModels[21];
double gWeightedBezierPoints[12][3];
double gBezierPoints[99][3];

void EvalBezier(int n, double (*points)[3], double *out, double t)
{
    double work[12][3];
    double s;
    double w;
    int i;
    int last;

    work[0][0] = points[0][0];
    work[0][1] = points[0][1];
    work[0][2] = points[0][2];
    last = n - 1;
    w = t;
    for (i = 1; i <= last; i++) {
        work[i][0] = points[i][0] * w;
        work[i][1] = points[i][1] * w;
        work[i][2] = points[i][2] * w;
        w *= t;
    }
    out[0] = work[last][0];
    out[1] = work[last][1];
    out[2] = work[last][2];
    s = 1.0 - t;
    w = s;
    for (i = last - 1; i >= 0; i--) {
        out[0] += work[i][0] * w;
        out[1] += work[i][1] * w;
        out[2] += work[i][2] * w;
        w *= s;
    }
}

void BeginFrame(void)
{
    GsSetRefView2(&gView);
    gActiveBuffer = GsGetActiveBuff();
    GsSetWorkBase(gPacketArea[gActiveBuffer]);
    GsClearOt(0, 0, &gOt[gActiveBuffer]);
    GsClearOt(0, 0, &gSubOt[gActiveBuffer]);
}

void SortModel(Model *model, GsOT ot)
{
    MATRIX localScreen;
    MATRIX localWorld;
    unsigned int i;
    GsDOBJ2 *object = &model->object;

    for (i = 0; i < model->objectCount; i++, object++) {
        GsGetLws(object->coord2, &localWorld, &localScreen);
        GsSetLightMatrix(&localWorld);
        GsSetLsMatrix(&localScreen);
        GsSortObject4(object, &ot, 0, getScratchAddr(0));
    }
}

void EndFrame(void)
{
    DrawSync(0);
    GsSortOt(&gSubOt[gActiveBuffer], &gOt[gActiveBuffer]);
    DrawSync(0);
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &gOt[gActiveBuffer]);
    GsDrawOt(&gOt[gActiveBuffer]);
}

void InitView(void)
{
    SetViewMode(VIEW_FRONT_RETURN);
    GsSetNearClip(0x64);
    GsSetFarClip(0x3e8);
}

u_char gViewReady;

void SetViewMode(int mode)
{
    GsSetProjection(350);
    mode &= 0xff;
    switch (mode) {
    case VIEW_TOP:
        gView = gTopView;
        gViewMode = mode;
        break;
    case VIEW_FRONT_RETURN:
        gView = gFrontView;
        gViewMode = mode;
        break;
    case VIEW_DESCENT:
        gView = gHighTopView;
        gViewMode = mode;
        break;
    default:
        gView = gFrontView;
        gViewMode = VIEW_FRONT_RETURN;
        break;
    }
    GsSetRefView2(&gView);
    gViewReady = 0xff;
}

int StartLightFade(int light, GsF_LIGHT *target)
{
    if (gLightFading == 0 ||
        (gLightFading == 1 && gFadeLightIndex == light)) {
        gFadeLightIndex = light;
        gFadeLightTarget = target;
        gLightFading = 1;
        return 0;
    }
    return 1;
}

void DisableFog(void)
{
    GsSetLightMode(0);
}

GsFOGPARAM gFogParam;

void EnableFog(int r, int g, int b, int dqa)
{
    gFogParam.dqa = dqa;
    gFogParam.dqb = 0x1400000;
    gFogParam.rfc = r;
    gFogParam.gfc = g;
    gFogParam.bfc = b;
    GsSetFogParam(&gFogParam);
    GsSetLightMode(1);
}

void InitModelCoordinate(Model *model)
{
    GsInitCoordinate2(WORLD, &model->coord);
    model->rot.vz = 0;
    model->rot.vy = 0;
    model->rot.vx = 0;
    model->coord.coord.t[2] = 0;
}

void UpdateModelMatrix(Model *model)
{
    MATRIX matrix = GsIDMATRIX;

    matrix.t[0] = model->coord.coord.t[0];
    matrix.t[1] = model->coord.coord.t[1];
    matrix.t[2] = model->coord.coord.t[2];
    RotMatrix(&model->rot, &matrix);
    model->coord.coord = matrix;
    model->coord.flg = 0;
}

void UpdateCoordinateMatrix(RotatedCoordinate *rotated)
{
    MATRIX matrix = GsIDMATRIX;

    matrix.t[0] = rotated->coord.coord.t[0];
    matrix.t[1] = rotated->coord.coord.t[1];
    matrix.t[2] = rotated->coord.coord.t[2];

    RotMatrix(&rotated->rot, &matrix);
    rotated->coord.coord = matrix;
    rotated->coord.flg = 0;
}

void UpdateCoordinatePairMatrices(void *data)
{
    RotatedCoordinatePair *pair = data;
    MATRIX matrix = GsIDMATRIX;

    matrix.t[0] = pair->coord[0].coord.t[0];
    matrix.t[1] = pair->coord[0].coord.t[1];
    matrix.t[2] = pair->coord[0].coord.t[2];
    RotMatrix(&pair->rot, &matrix);
    pair->coord[0].coord = matrix;
    pair->coord[0].flg = 0;

    matrix = GsIDMATRIX;
    matrix.t[0] = pair->coord[1].coord.t[0];
    matrix.t[1] = pair->coord[1].coord.t[1];
    matrix.t[2] = pair->coord[1].coord.t[2];
    RotMatrix(&pair->rot, &matrix);
    pair->coord[1].coord = matrix;
    pair->coord[1].flg = 0;
}

void UploadTim(u_long *tim)
{
    RECT rect;
    GsIMAGE image;
    u_long *data;
    short x, y;
    u_short w, h;

    GsGetTimInfo(tim + 1, &image);
    data = image.pixel;
    x = image.px;
    y = image.py;
    w = image.pw;
    h = image.ph;
    setRECT(&rect, x, y, w, h);
    LoadImage(&rect, data);
    if ((image.pmode >> 3) & 1) {
        data = image.clut;
        x = image.cx;
        y = image.cy;
        w = image.cw;
        h = image.ch;
        setRECT(&rect, x, y, w, h);
        LoadImage(&rect, data);
    }
}

void SetModelTmd(u_long *tmd, Model *model)
{
    unsigned int count;

    ++tmd;
    GsMapModelingData(tmd);
    ++tmd;
    count = *tmd++;
    model->objects = (struct TMD_STRUCT *)tmd;
    model->objectCount = count;
}

void LinkModelObjects(Model *model, GsCOORDINATE2 *parent)
{
    GsDOBJ2 *object;
    struct TMD_STRUCT *objects;
    unsigned int count, i;

    model->firstObjectIndex = gLinkedObjectCount;
    objects = model->objects;
    for (i = 0; i < model->objectCount; i++) {
        GsLinkObject4((u_long)objects, &model->object, i);
    }
    object = &model->object;
    count = model->objectCount;
    gLinkedObjectCount += i;
    i = 0;
    if (count != 0) {
        do {
            object->coord2 = &model->coord;
            object++->attribute = 0;
            ++i;
        } while (i < model->objectCount);
    }
    if (parent != 0) {
        GsInitCoordinate2(parent, &model->coord);
    }
}

void InitChildCoordinate(GsCOORDINATE2 *coord, GsCOORDINATE2 *parent)
{
    if (parent != 0) {
        GsInitCoordinate2(parent, coord);
    }
}

void InitChildCoordinatePair(void *data, GsCOORDINATE2 *parent)
{
    RotatedCoordinatePair *pair = data;

    if (parent != 0) {
        GsInitCoordinate2(parent, &pair->coord[0]);
        GsInitCoordinate2(parent, &pair->coord[1]);
    }
}

int ProjectModelToScreen(Model *model, DVECTOR *screen)
{
    MATRIX localWorld;
    MATRIX localScreen;
    long distance;
    int y;

    GsGetLws(&model->coord, &localWorld, &localScreen);
    model->coord.flg = 0;
    distance = ReadGeomScreen();
    if (localScreen.t[2] == 0) {
        localScreen.t[2] = 40;
    }
    screen->vx = localScreen.t[0] * distance / localScreen.t[2] + 160;
    y = localScreen.t[1] * distance / localScreen.t[2] + 120;
    screen->vy = y;
    return y;
}

void ProjectCoordinateToScreen(GsCOORDINATE2 *coord, SVECTOR *screen)
{
    MATRIX localWorld;
    MATRIX localScreen;
    long distance;

    GsGetLws(coord, &localWorld, &localScreen);
    coord->flg = 0;
    distance = ReadGeomScreen();
    if (localScreen.t[2] == 0) {
        localScreen.t[2] = 40;
    }
    screen->vx = localScreen.t[0] * distance / localScreen.t[2] + 160;
    screen->vy = localScreen.t[1] * distance / localScreen.t[2] + 120;
}

void ProjectCoordinatePairToScreen(GsCOORDINATE2 *coord, short *from, short *to)
{
    MATRIX localWorld;
    MATRIX localScreen;
    long distance;

    GsGetLws(&coord[0], &localWorld, &localScreen);
    coord[0].flg = 0;
    distance = ReadGeomScreen();
    if (localScreen.t[2] == 0) {
        localScreen.t[2] = 40;
    }
    from[0] = localScreen.t[0] * distance / localScreen.t[2] + 160;
    from[1] = localScreen.t[1] * distance / localScreen.t[2] + 120;
    GsGetLws(&coord[1], &localWorld, &localScreen);
    coord[1].flg = 0;
    distance = ReadGeomScreen();
    if (localScreen.t[2] == 0) {
        localScreen.t[2] = 40;
    }
    to[0] = localScreen.t[0] * distance / localScreen.t[2] + 160;
    to[1] = localScreen.t[1] * distance / localScreen.t[2] + 120;
}

void SetTmdVertex(int index, const SVECTOR *vertex, const struct TMD_STRUCT *object)
{
    ((SVECTOR *)object->vertop)[index] = *vertex;
}

void GetTmdVertex(int index, SVECTOR *vertex, const struct TMD_STRUCT *object)
{
    *vertex = ((SVECTOR *)object->vertop)[index];
}

void SetTmdNormal(int index, const SVECTOR *normal, const struct TMD_STRUCT *object)
{
    ((SVECTOR *)object->nortop)[index] = *normal;
}

void GetTmdNormal(int index, SVECTOR *normal, const struct TMD_STRUCT *object)
{
    *normal = ((SVECTOR *)object->nortop)[index];
}

void WeightBezierPoints(int n, double (*points)[3], double (*out)[3])
{
    int i;
    int last;
    int coefficient;
    double weight;

    last = n - 1;
    for (i = 0; i <= last; i++) {
        if (i == 0) {
            coefficient = 1;
        } else if (i == 1) {
            coefficient = last;
        } else {
            coefficient = coefficient * (last - i + 1) / i;
        }
        weight = coefficient;
        out[i][0] = points[i][0] * weight;
        out[i][1] = points[i][1] * weight;
        out[i][2] = points[i][2] * weight;
    }
}

int gPointDeltaStepsLeft;
int gPointDeltaCount;
SVECTOR gPointDeltas[256];

void InitPointDeltas(const SVECTOR *target, const SVECTOR *points, short steps, unsigned int count)
{
    unsigned int i = 0;
    SVECTOR *delta;

    if (count != 0) {
        delta = gPointDeltas;
        do {
            delta[i].vx = (target[i].vx - points[i].vx) / steps;
            delta[i].vy = (target[i].vy - points[i].vy) / steps;
            delta[i].vz = (target[i].vz - points[i].vz) / steps;
        } while (++i < count);
    }
    gPointDeltaStepsLeft = steps;
    gPointDeltaCount = count;
}

int ApplyPointDeltas(SVECTOR *points)
{
    int count = gPointDeltaCount;
    int i = 0;

    if (count > 0) {
        do {
            points[i].vx += gPointDeltas[i].vx;
            points[i].vy += gPointDeltas[i].vy;
            points[i].vz += gPointDeltas[i].vz;
        } while (++i < count);
    }
    return --gPointDeltaStepsLeft;
}

extern u_long gLainIconTim[];
extern u_long gLainDarkIconTim[];
extern u_long gToukoIconTim[];
extern u_long gLabIconTim[];
extern u_long gSaiseikunIconTim[];
extern u_long gTransferIconTim[];
extern u_long gMovieIconTim[];
extern u_long gPsychiatryIconTim[];
extern u_long gOpenedLainIconTim[];
extern u_long gOpenedLainDarkIconTim[];
extern u_long gOpenedToukoIconTim[];
extern u_long gOpenedLabIconTim[];
extern u_long gOpenedSaiseikunIconTim[];
extern u_long gOpenedTransferIconTim[];
extern u_long gOpenedMovieIconTim[];
extern u_long gOpenedPsychiatryIconTim[];

GsSPRITE gClipFrameSprites[4] = {
    {
        0x5a000000, 0, 0, 0x00000100, 0x000000f0, 0x00000000, 0x00000000,
        0x00000000, 0, 0, 0x00000080, 0x00000080, 0x00000080, 1,
        1, 0, 0, 0,
    },
    {
        0x5a000000, 0, 0, 0x00000040, 0x000000f0, 0x00000000, 0x00000000,
        0x00000000, 0, 0, 0x00000080, 0x00000080, 0x00000080, 1,
        1, 0, 0, 0,
    },
    {
        0x5a000000, 0, 0, 0x00000100, 0x000000f0, 0x00000000, 0x00000000,
        0x00000000, 0, 0, 0x00000080, 0x00000080, 0x00000080, 1,
        1, 0, 0, 0,
    },
    {
        0x5a000000, 0, 0, 0x00000040, 0x000000f0, 0x00000000, 0x00000000,
        0x00000000, 0, 0, 0x00000080, 0x00000080, 0x00000080, 1,
        1, 0, 0, 0,
    },
};
u_long *gCellTmds[16] = {
    (void *)&gLainCellTmd, (void *)&gLainDarkCellTmd, (void *)&gToukoCellTmd, (void *)&gLabCellTmd,
    (void *)&gSaiseikunCellTmd, (void *)&gTransferCellTmd, (void *)&gMovieCellTmd, (void *)&gPsychiatryCellTmd,
    (void *)&gOpenedLainCellTmd, (void *)&gOpenedLainDarkCellTmd, (void *)&gOpenedToukoCellTmd, (void *)&gOpenedLabCellTmd,
    (void *)&gOpenedSaiseikunCellTmd, (void *)&gOpenedTransferCellTmd, (void *)&gOpenedMovieCellTmd, (void *)&gOpenedPsychiatryCellTmd,
};
u_long *gCellTims[16] = {
    (void *)gLainIconTim, (void *)gLainDarkIconTim, (void *)gToukoIconTim, (void *)gLabIconTim,
    (void *)gSaiseikunIconTim, (void *)gTransferIconTim, (void *)gMovieIconTim, (void *)gPsychiatryIconTim,
    (void *)gOpenedLainIconTim, (void *)gOpenedLainDarkIconTim, (void *)gOpenedToukoIconTim, (void *)gOpenedLabIconTim,
    (void *)gOpenedSaiseikunIconTim, (void *)gOpenedTransferIconTim, (void *)gOpenedMovieIconTim, (void *)gOpenedPsychiatryIconTim,
};
u_long *gCellPieceTmds[4] = { (u_long *)&gCellPiece0Tmd, (u_long *)&gCellPiece1Tmd, (u_long *)&gCellPiece2Tmd, (u_long *)&gCellPiece3Tmd };
u_long *gBurstPieceTmds[32] = {
    (void *)&gBurstPiece00Tmd, (void *)&gBurstPiece01Tmd, (void *)&gBurstPiece02Tmd, (void *)&gBurstPiece03Tmd,
    (void *)&gBurstPiece04Tmd, (void *)&gBurstPiece05Tmd, (void *)&gBurstPiece06Tmd, (void *)&gBurstPiece07Tmd,
    (void *)&gBurstPiece08Tmd, (void *)&gBurstPiece09Tmd, (void *)&gBurstPiece10Tmd, (void *)&gBurstPiece11Tmd,
    (void *)&gBurstPiece12Tmd, (void *)&gBurstPiece13Tmd, (void *)&gBurstPiece14Tmd, (void *)&gBurstPiece15Tmd,
    (void *)&gBurstPiece16Tmd, (void *)&gBurstPiece17Tmd, (void *)&gBurstPiece18Tmd, (void *)&gBurstPiece19Tmd,
    (void *)&gBurstPiece20Tmd, (void *)&gBurstPiece21Tmd, (void *)&gBurstPiece22Tmd, (void *)&gBurstPiece23Tmd,
    (void *)&gBurstPiece24Tmd, (void *)&gBurstPiece25Tmd, (void *)&gBurstPiece26Tmd, (void *)&gBurstPiece27Tmd,
    (void *)&gBurstPiece28Tmd, (void *)&gBurstPiece29Tmd, (void *)&gBurstPiece30Tmd, (void *)&gBurstPiece31Tmd,
};
GsRVIEW2 gFrontView = { 0, 0, -1750, 0, 0, 0, 0, 0 };
GsRVIEW2 gTopView = { 0, -2700, 0, 0, 0, 0, 0, 0 };
GsRVIEW2 gFrontTopPathPoint = { 0, -2700, -2700, 0, 0, 0, 0, 0 };
GsRVIEW2 gHighTopView = { 0, -17000, 0, 0, 0, 0, 0, 0 };
GsRVIEW2 gHighFrontPathPoint = { 0, -2000, -750, 0, 0, 0, 0, 0 };
GsF_LIGHT gFrontViewLights[3] = {
    { 0, 0, 700, 0xff, 0xff, 0xff },
    { 0, 1000, 0, 0x7f, 0x7f, 0x7f },
    { 0, 0, 0, 0, 0, 0 },
};
GsF_LIGHT gTopViewLights[3] = {
    { 700, 0, 0, 0xff, 0xff, 0xff },
    { 0, 1000, 0, 0x7f, 0x7f, 0xff },
    { -700, 0, 0, 0xff, 0xff, 0xff },
};
VECTOR gFlightPathEnds[5] = {
    { 152, 10, -300, 0 },
    { 152, 10, -400, 0 },
    { -200, 100, -500, 0 },
    { 345, 255, -500, 0 },
    { 345, 255, -500, 0 },
};
VECTOR gFlightPathControls[5] = {
    { 500, 0, -800, 0 },
    { 500, 0, -800, 0 },
    { -500, 0, -800, 0 },
    { 500, 0, -800, 0 },
    { 500, 0, -800, 0 },
};
double gFlightStepCounts[5] = { 14.0, 14.0, 8.0, 4.0, 4.0 };
SVECTOR gClipSet0FlightEndPath[10] = {
    { 152, 10, -300, 0 },
    { 152, 10, -300, 0 },
    { 152, 10, -300, 0 },
    { 165, 15, -300, 0 },
    { 170, 15, -300, 0 },
    { 182, 18, -300, 0 },
    { 182, 22, -300, 0 },
    { 182, 30, -300, 0 },
    { 181, 28, -300, 0 },
    { 180, 28, -300, 0 },
};
SVECTOR gClipSet1FlightEndPath[15] = {
    { 152, 10, -400, 0 },
    { 152, 10, -400, 0 },
    { 152, 10, -400, 0 },
    { 140, 10, -400, 0 },
    { 130, 10, -400, 0 },
    { 120, 20, -400, 0 },
    { 85, 50, -400, 0 },
    { 65, 70, -400, 0 },
    { 40, 80, -400, 0 },
    { 25, 90, -400, 0 },
    { 25, 90, -400, 0 },
    { 25, 90, -400, 0 },
    { 25, 90, -400, 0 },
    { 25, 90, -400, 0 },
    { 25, 90, -400, 0 },
};
SVECTOR gCellPieceSteps[4] = {
    { 20, 0, 150, 0 },
    { -20, 0, 150, 0 },
    { 150, 0, -50, 0 },
    { -150, 0, -50, 0 },
};
short gPathStreakHeights[16] = {
    -2300, -2000, -1800, -1500, -1300, -1000, -800, -500, 500, 800, 1000, 1300, 1500,
    1800, 2000, 2300,
};
short gVerticalStreakXs[16] = {
    2300, 2000, 1800, 1500, 1300, 1000, 800, 500, -500, -800, -1000, -1300, -1500,
    -1800, -2000, -2300,
};
short gVerticalStreakZs[16] = {
    2300, 2000, 1800, 1500, 1300, 1000, 800, 500, -500, -800, -1000, -1300, -1500,
    -1800, -2000, -2300,
};
/* Straight streak paths across the grid: [0] is where a streak ends, [1] where it starts. */
SVECTOR gNearStreakPathNegXNegZ[2] = {
    { -8571, 3000, -5571, 0 },
    { 5571, 3000, 8571, 0 },
};
SVECTOR gNearStreakPathPosXNegZ[2] = {
    { 8571, 3000, -5571, 0 },
    { -5571, 3000, 8571, 0 },
};
SVECTOR gFarStreakPathNegXNegZ[2] = {
    { -10071, 3000, -4071, 0 },
    { 4071, 3000, 10071, 0 },
};
SVECTOR gFarStreakPathPosXNegZ[2] = {
    { 10071, 3000, -4071, 0 },
    { -4071, 3000, 10071, 0 },
};
SVECTOR gCellFaceNormals[6] = {
    { -4096, 0, 0, 0 },
    { 0, 0, -4096, 0 },
    { 0, -4096, 0, 0 },
    { 0, 0, 4096, 0 },
    { 0, 4096, 0, 0 },
    { 4096, 0, 0, 0 },
};
SVECTOR gLevelLabelPositions[2] = {
    { 0, 0, -661, 0 },
    { 0, 0, 661, 0 },
};
SVECTOR gSmallSpherePositions[5] = {
    { -365, 264, -370, 0 },
    { -185, -300, -615, 0 },
    { 330, 59, 524, 0 },
    { 460, 270, -25, 0 },
    { 365, -455, 159, 0 },
};
SVECTOR gGateModelPositions[4] = {
    { 0, 0, -150, 0 },
    { -150, 0, 0, 0 },
    { 0, 0, 150, 0 },
    { 150, 0, 0, 0 },
};
u_long *gGateSlabTmds[5] = {
    (void *)&gGateSlab0Tmd, (void *)&gGateSlab1Tmd, (void *)&gGateSlab2Tmd, (void *)&gGateSlab3Tmd,
    (void *)&gGateFlyingSlabTmd,
};

int gPointDeltaMode;

PACKET gPacketArea[2][36000];

GsOT gSubOt[2];

GsOT gOt[2];

GsRVIEW2 gView;

GsF_LIGHT gFlatLights[3];
