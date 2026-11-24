#ifndef GRID_NODES_H
#define GRID_NODES_H

/* One cell of the node grid, 24 to a level. */
typedef struct {
    short nodeIndex;
    short modelType;
    int modelIndex;
    short screenX;
    short screenY;
} GridCell;

extern int gGridHeading;
extern int gGridBusy;
extern int gGridLevel;

int IsNodeVisible(int id);
void BuildNodeGrid(void);
int FindNodeSharingWord();
void MarkNodeOpened(int id, int soundPlayed);
int PickCursorClipSet(void);
int IsCursorNodeMedia(void);
void ResetNodeStates(void);

#endif
