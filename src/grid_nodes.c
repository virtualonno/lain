/* Grid node placement and unlock state: maps the 716 nodes onto disc levels and 24-cell slots,
 * tracks opened flags, and looks up linked nodes sharing a word. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include <memory.h>
#include "nodes.h"
#include "grid_menu.h"
#include "grid_nodes.h"
#include "grid_scene.h"
#include "lapks_player.h"
#include "sound.h"

int gGridHeading;

static inline void FindNodeCell(int id, short *outCell, short *outLevel)
{
    int level;
    int cell;

    for (level = 0; level < GetGridLevelCount(); level++) {
        for (cell = 0; cell < 24; cell++) {
            if (gGridCells[level][cell].nodeIndex == id) {
                *outCell = cell;
                *outLevel = level;
                return;
            }
        }
    }
    *outCell = -1;
    *outLevel = -1;
}

int gGridBusy;
int gGridLevel;
/* Maps Node kind (flags bits 20..23) to gCellTmds/gCellTims index (+8 when opened):
 * 0:Env/Lda 2:Tda 3:Cou 4:Dc 5:Dia 6:Eda/Ekm/Ere/TaK 7:SSkn 8:GaTE 9:P2-; no node has kind 1, 10 or 11. */
short gNodeKindModelTypes[12] = { 0, 1, 2, 3, 6, 7, 5, 4, 5, 5, 5, 0 };

/* Nonzero gEndRollCount allows nodes up to threshold gEndRollCount + 1 to appear on the grid. */
int IsNodeVisible(int id)
{
    Node *table = gNodes;
    Node *node = &table[id];
    unsigned int flags;
    short progress;

    if (((u_char *)&node->flags)[3] & 1) {
        if (gCurrentDisc != 1 && (((u_char *)&node->gridPos)[1] >> 7) != 0) {
            return 0;
        }
        flags = gNodes[id].flags;
        if (((flags >> 20) & 15) == NODE_KIND_UPGRADE && (int)((flags >> 16) & 15) < gUpgradesApplied) {
            return 0;
        }
        progress = gEndRollCount;
        if (progress != 0) {
            int threshold = gNodes[id].flags >> 28;
            if (progress + 1 >= threshold) {
                return 1;
            }
        } else if (progress >= (int)(gNodes[id].flags >> 28)) {
            return 1;
        }
    }
    return 0;
}

void BuildNodeGrid(void)
{
    int i;
    int j;
    u_short gridPos;
    int row;
    int level;
    int cell;

    gGridHeading = 0;
    gGridLevel = 0;
    gGridBusy = 0;
    for (i = 0; i < GetGridLevelCount(); i++) {
        memset(gGridCells[i], 0, sizeof(gGridCells[i]));
    }
    for (i = 0; i < GetGridLevelCount(); i++) {
        for (j = 0; j < 24; j++) {
            gGridCells[i][j].nodeIndex = -1;
        }
    }
    for (i = 0; i < 716; i++) {
        gridPos = gNodes[i].gridPos;
        cell = gridPos & 7;
        row = (short)gridPos >> 3 & 0xfff;
        if (((short)gridPos < 0) == gCurrentDisc && row > 0) {
            row--;
            level = row / 3;
            cell += row % 3 * 8;
            if (IsNodeVisible(i) != 0) {
                gGridCells[level][cell].nodeIndex = i;
                gGridCells[level][cell].modelType = gNodeKindModelTypes[(gNodes[i].flags >> 20) & 0xf];
                if ((gNodes[i].flags >> 26) & 1) {
                    gGridCells[level][cell].modelType += 8;
                }
            }
        }
    }
}

int FindNodeSharingWord(start, slot, outCell, outLevel)
short start, slot;
short *outCell, *outLevel;
{
    int key;
    int id;
    int k;

    key = gNodes[start].words[slot];
    for (id = start + 1; id < 716; id++) {
        if (IsNodeVisible(id) != 0) {
            for (k = 0; k < 3; k++) {
                if (gNodes[id].words[k] == key) {
                    FindNodeCell(id, outCell, outLevel);
                    if (*outCell != -1) {
                        return 1;
                    }
                }
            }
        }
    }
    for (id = 0; id < start; id++) {
        if (IsNodeVisible(id) != 0) {
            for (k = 0; k < 3; k++) {
                if (gNodes[id].words[k] == key) {
                    FindNodeCell(id, outCell, outLevel);
                    if (*outCell != -1) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void MarkNodeOpened(int id, int soundPlayed)
{
    int i;
    Node *node;
    short gridPos;
    int disc;
    int cell;
    int level;
    int row;

    if (((gNodes[id].flags >> 26) & 1) == 0) {
        gNodes[id].flags |= 0x4000000;
        for (i = 0; i < 716; i++) {
            node = &gNodes[i];
            if (node->unlockedBy == id) {
                gridPos = node->gridPos;
                cell = gridPos & 7;
                row = (gridPos >> 3) & 0xfff;
                disc = (gridPos >> 15) & 1;
                if (row > 0) {
                    node->flags |= 0x1000000;
                    if (disc == gCurrentDisc) {
                        row--;
                        level = row / 3;
                        cell += (row % 3) * 8;
                        if (gGridCells[level][cell].nodeIndex < 0 && IsNodeVisible(i) != 0) {
                            if (soundPlayed == 0) {
                                PlaySound(31);
                                soundPlayed = 1;
                            }
                            gGridCells[level][cell].nodeIndex = i;
                            gGridCells[level][cell].modelType = gNodeKindModelTypes[(node->flags >> 20) & 0xf];
                            RebuildCellModel(level, cell);
                        }
                    }
                }
            }
        }
        if (((gNodes[id].flags >> 20) & 0xf) - NODE_KIND_UPGRADE < 3) {
            gNodes[id].flags &= ~0x1000000;
        }
    }
}

int PickCursorClipSet(void)
{
    short level = gCursorRow / 3;
    short rowInLevel = gCursorRow % 3;
    int entry = gCursorColumn + rowInLevel * 8;
    entry = gGridCells[level][entry].nodeIndex;

    if (entry >= 0) {
        unsigned int flags = gNodes[entry].flags;
        if (gEndRollCount >= (int)(flags >> 28)) {
            int required = (flags >> 16) & 15;
            if (required <= gUpgradesApplied) {
                return rand() % 2;
            }
        }
        return rand() % 3 + 2;
    }
    return -1;
}

int IsCursorNodeMedia(void)
{
    short level = gCursorRow / 3;
    short rowInLevel = gCursorRow % 3;
    int index = gCursorColumn + rowInLevel * 8;
    index = gGridCells[level][index].nodeIndex;
    return (int)((gNodes[index].flags >> 20) & 15) < NODE_KIND_UPGRADE;
}

void ResetNodeStates(void)
{
    int i;

    for (i = 0; i < 716; ++i) {
        unsigned int flags = gNodes[i].flags & ~(1U << 26);
        gNodes[i].flags = (flags & ~(1U << 24)) | (((flags >> 25) & 1) << 24);
    }
}
