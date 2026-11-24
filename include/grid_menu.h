#ifndef GRID_MENU_H
#define GRID_MENU_H

extern short gGatesOpened;
extern short gP2NodesOpened;
extern short gUpgradesApplied;
extern short gEndRollCount;
extern short gCursorColumn;
extern short gCursorRow;
extern short gOtherDiscCursorColumn;
extern short gOtherDiscCursorRow;
extern short gGakkuriPercent;
extern short gHarumagePercent;
extern short gTokimekiPercent;
extern short gCurrentDisc;
extern short gDokudenpaCount;
extern short gColumnOrder[8];
extern short gColumnRightOf[8];
extern short gColumnLeftOf[8];
extern short gPreviousCursorRow;
extern short gCursorOnNode;
extern int gLoadCount;

void CopyCellNodeText();
int SnapNextCursorToNode(int columnMode, int rowMode);
int MoveGridCursor(int dx, int dy);
void UpdateCursorDuringMove(int commit);
void HandleGridInput(void);
void InitJumpToPanel(void);
void SlideJumpToSpritesIn(void);
void SlideJumpToSpritesOut(void);
void SetJumpToDigits(int level);
void DrawJumpToSprites(void);
void EndSceneFrame(void);
int GetGridLevelCount(void);
void InitGridState(void);
void RestoreGridCursor(void);
enum GridMenuResult {
    GRID_MENU_EXIT_START,
    GRID_MENU_EXIT_LOAD,
    GRID_MENU_EXIT_SITE_CHANGE,
    GRID_MENU_EXIT_CONTINUE
};

int RunGridMenu(void);

#endif
