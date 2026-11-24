#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* The GsDOBJ2 objects linked from one TMD (see LinkModelObjects), placed by one coordinate. */
typedef struct Model {
    GsDOBJ2 object;
    GsCOORDINATE2 coord;
    SVECTOR rot;
    u_long objectCount;
    struct TMD_STRUCT *objects;
    int firstObjectIndex;
} Model;

typedef struct {
    GsCOORDINATE2 coord;
    SVECTOR rot;
} RotatedCoordinate;

typedef struct {
    GsCOORDINATE2 coord[2];
    SVECTOR rot;
} RotatedCoordinatePair;

extern Model gModels[51];
extern Model gCellModels[103];
extern Model gLevelModels[21];

extern int gActiveBuffer;
extern short gClutFading;
extern u_char gLightFading;
/* SetViewMode presets gFrontView via VIEW_FRONT_RETURN; StepViewPath and StepViewDescent switch to VIEW_FRONT once settled. */
enum ViewMode {
    VIEW_FRONT = 1,
    VIEW_TOP,
    VIEW_FRONT_RETURN,
    VIEW_DESCENT
};
extern u_char gViewMode;
extern double gWeightedBezierPoints[12][3];
extern double gBezierPoints[99][3];
extern u_char gViewReady;
extern int gPointDeltaStepsLeft;
extern int gPointDeltaCount;
extern GsSPRITE gClipFrameSprites[4];
extern u_long *gCellTmds[16];
extern u_long *gCellTims[16];
extern u_long *gCellPieceTmds[4];
extern u_long *gBurstPieceTmds[32];
extern GsRVIEW2 gFrontView;
extern GsRVIEW2 gTopView;
extern GsRVIEW2 gFrontTopPathPoint;
extern GsRVIEW2 gHighTopView;
extern GsRVIEW2 gHighFrontPathPoint;
extern GsF_LIGHT gFrontViewLights[3];
extern GsF_LIGHT gTopViewLights[3];
extern VECTOR gFlightPathEnds[5];
extern VECTOR gFlightPathControls[5];
extern double gFlightStepCounts[5];
extern SVECTOR gClipSet0FlightEndPath[10];
extern SVECTOR gClipSet1FlightEndPath[15];
extern SVECTOR gCellPieceSteps[4];
extern short gPathStreakHeights[16];
extern short gVerticalStreakXs[16];
extern short gVerticalStreakZs[16];
extern SVECTOR gNearStreakPathNegXNegZ[2];
extern SVECTOR gNearStreakPathPosXNegZ[2];
extern SVECTOR gFarStreakPathNegXNegZ[2];
extern SVECTOR gFarStreakPathPosXNegZ[2];
extern SVECTOR gCellFaceNormals[6];
extern SVECTOR gLevelLabelPositions[2];
extern SVECTOR gSmallSpherePositions[5];
extern SVECTOR gGateModelPositions[4];
extern u_long *gGateSlabTmds[5];
enum PointDeltaMode {
    POINT_DELTA_NONE,
    POINT_DELTA_RING_FLAT,
    POINT_DELTA_RING_BENT_DOWN,
    POINT_DELTA_RING_BENT_UP,
    POINT_DELTA_RING_DIP,
    POINT_DELTA_CELL_FLIGHT,
    POINT_DELTA_SPHERE_COLLAPSE
};
extern int gPointDeltaMode;
extern PACKET gPacketArea[2][36000];
extern GsOT gSubOt[2];
extern GsOT gOt[2];
extern GsRVIEW2 gView;
extern GsF_LIGHT gFlatLights[3];

int InitGraphics(void);
void SetViewLights(void);
void StepLightFade(void);
void StepClutFade(u_long *image, u_long *target);
void BeginFrame(void);
void SortModel(Model *model, GsOT ot);
void EndFrame(void);
void InitView(void);
void SetViewMode(int mode);
void DisableFog(void);
void InitModelCoordinate(Model *model);
void UpdateModelMatrix(Model *model);
void UpdateCoordinatePairMatrices(void *data);
void UploadTim(u_long *tim);
void SetModelTmd(u_long *tmd, Model *model);
void LinkModelObjects(Model *model, GsCOORDINATE2 *parent);
void InitChildCoordinatePair(void *data, GsCOORDINATE2 *parent);
void ProjectCoordinatePairToScreen(GsCOORDINATE2 *coord, short *from, short *to);
void InitPointDeltas(const SVECTOR *target, const SVECTOR *points, short steps, unsigned int count);
void EvalBezier(int n, double (*points)[3], double *out, double t);
int ProjectModelToScreen(Model *model, DVECTOR *screen);
void WeightBezierPoints(int n, double (*points)[3], double (*out)[3]);
int ApplyPointDeltas(SVECTOR *points);
void SetTmdVertex(int index, const SVECTOR *vertex, const struct TMD_STRUCT *object);
void GetTmdVertex(int index, SVECTOR *vertex, const struct TMD_STRUCT *object);
void SetTmdNormal(int index, const SVECTOR *normal, const struct TMD_STRUCT *object);

extern SVECTOR gPointDeltas[256];

#endif
