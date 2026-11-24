#ifndef GRID_SCENE_H
#define GRID_SCENE_H

#include <sys/types.h>
#include <libgte.h>

extern unsigned char gRingSettleSoundPending;
extern int gSceneAnimation;
extern int gViewTiltedUp;
extern int gViewTiltedDown;
extern short gTurnStep;
extern int gColumnsToTurn;
extern short gClipFrameOffsetX;
extern short gTiltOffsetY;
extern u_char gCellFlying;
extern u_char gFlightEnding;
extern int gSavedPointDeltaMode;
extern int gSavedPointDeltaStepsLeft;
extern double gViewPathStep;
extern short gFlashLevel;
extern short gFlashFramesLeft;
extern int gHighlightSelection[2];
extern int gSineTable[2560];
extern SVECTOR gCellPositions[24];
extern SVECTOR gFlatCellPoints[8];
extern SVECTOR gCubeCellPoints[8];
extern SVECTOR gCollapsedCellPoints[8];
extern SVECTOR gRingPoints[64];

int IsPointOffScreen(const short *point, short margin);
void InitGridScene(void);
void BuildCellModel(unsigned int level, unsigned int cell);
void RebuildCellModel(unsigned int level, unsigned int cell);
void BuildLevelModels(int group, int level, int firstYaw, int secondYaw);
void UpdateGridScene(void);
void StepLevelTravel(void);
void StepTravelToCursor(void);
void DrawClipFrame(void);
void StepClipFrameFlash(void);
void ProjectCellsToScreen(void);
void SortGridScene(void);
void SetLevelNumberTexture(int index);
void StartViewPath(u_int mode);
void StepViewPath(void);
void SortCursorHighlight(void);
void StartCellFlight(void);
void StepCellFlight(void);
void StepCellFlightEnd(void);
void InitClipFrameSprites(void);
void ResetGridSceneToCell(unsigned int level, unsigned int cell);
void StepOrFinishTravelToCursor(void);
void StartViewDescent(void);
void StepViewDescent(void);

extern SVECTOR gSavedPointDeltas[256];

#endif
