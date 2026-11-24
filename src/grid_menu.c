/* Main grid menu state machine (enum GridMenuState): handles cursor navigation, Jump To panel,
 * and opening every node screen (media, voice, gate, P2, upgrade, system menu, continue prompt).
 * Owns the cursor, the progress counters and the Jump To sprites. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <strings.h>
#include "loading.h"
#include "nodes.h"
#include "graphics.h"
#include "grid_effects.h"
#include "grid_menu.h"
#include "grid_nodes.h"
#include "grid_scene.h"
#include "lapks_player.h"
#include "media_player.h"
#include "memcard.h"
#include "pad.h"
#include <libgs.h>
#include <libetc.h>
#include <libcd.h>
#include <libpress.h>
#include <libspu.h>
#include <rand.h>
#include <memory.h>
#include <libsnd.h>
#include <stdio.h>
#include <malloc.h>
#include <libmcrd.h>
#include <libsn.h>
#include "sound.h"
#include "string_tables.h"
#include "system_menu.h"
#include "text.h"
#include "continue_prompt.h"
#include "upgrade.h"


enum JumpToSprite {
    JUMP_TO_SPRITE_TENS,
    JUMP_TO_SPRITE_ONES,
    JUMP_TO_SPRITE_UP_ARROW,
    JUMP_TO_SPRITE_DOWN_ARROW,
    JUMP_TO_SPRITE_BAR = 5
};

GsSPRITE gJumpToSprites[8] = {
    { 0, 229, 100, 24, 32, 29, 48, 24, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 253, 100, 24, 32, 29, 48, 24, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 242, 75, 24, 16, 29, 96, 64, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 242, 132, 24, 16, 29, 120, 64, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 233, 92, 40, 8, 29, 96, 56, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 0, 104, 236, 24, 29, 0, 0, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 229, 64, 48, 176, 29, 0, 24, 0, 486, 128, 128, 128, 0, 0, 4096, 4096, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

short gGatesOpened = 0;
short gP2NodesOpened = 0;
short gUpgradesApplied = 0;
short gEndRollCount = 0;
enum GridMenuState {
    GRID_MENU_REST,
    GRID_MENU_TILTED_UP,
    GRID_MENU_TILTED_DOWN,
    GRID_MENU_OPEN_NODE,
    GRID_MENU_SYSTEM_MENU,
    GRID_MENU_JUMP_TO,
    GRID_MENU_TILT_UP,
    GRID_MENU_UNTILT_FROM_UP,
    GRID_MENU_TILT_DOWN,
    GRID_MENU_UNTILT_FROM_DOWN,
    GRID_MENU_TURN_LEFT,
    GRID_MENU_TURN_RIGHT = 13,
    GRID_MENU_LEVEL_UP = 16,
    GRID_MENU_LEVEL_DOWN,
    GRID_MENU_UNTILT_UP_LEVEL_UP,
    GRID_MENU_UNTILT_UP_LEVEL_DOWN,
    GRID_MENU_UNTILT_DOWN_LEVEL_UP,
    GRID_MENU_UNTILT_DOWN_LEVEL_DOWN,
    GRID_MENU_UNTILT_UP_OPEN_NODE,
    GRID_MENU_UNTILT_DOWN_OPEN_NODE,
    GRID_MENU_UNTILT_UP_SYSTEM_MENU,
    GRID_MENU_UNTILT_DOWN_SYSTEM_MENU,
    GRID_MENU_ENTER,
    GRID_MENU_IDLE_MEDIA,
    GRID_MENU_LINK_TRAVEL,
    GRID_MENU_NO_SHARED_NODE
};
short gGridMenuState;
short gGridMenuReturnState;
enum JumpToStep {
    JUMP_TO_INIT,
    JUMP_TO_SLIDE_IN,
    JUMP_TO_CHOOSE,
    JUMP_TO_SLIDE_OUT,
    JUMP_TO_CHECK_TRAVEL,
    JUMP_TO_LOAD_CLIP,
    JUMP_TO_START_TRAVEL,
    JUMP_TO_WAIT_TRAVEL,
    JUMP_TO_FINISH
};
enum LinkTravelStep {
    LINK_TRAVEL_MOVE,
    LINK_TRAVEL_FLASH
};
enum SystemMenuStep {
    SYSTEM_MENU_LOAD_CLIP,
    SYSTEM_MENU_WAIT_BURST,
    SYSTEM_MENU_START_RISE,
    SYSTEM_MENU_WAIT_RISE,
    SYSTEM_MENU_OPEN,
    SYSTEM_MENU_RUN,
    SYSTEM_MENU_START_CLOSE,
    SYSTEM_MENU_CLOSE,
    SYSTEM_MENU_WAIT_RETURN
};
short gGridMenuStep;
short gOpenClipSet;
short gTiltToggle;
short gJumpToLevel;
short gJumpToLevelChanged;
short gMoveStartLevel;
short gSkipInputFrames;
short gClipLoadRequested;
short gShowClipFrame;
short gIdleFrames;
short gLinkedNodeLevel;
short gLinkedNodeCell;
short gCursorColumn;
short gCursorRow;
short gOtherDiscCursorColumn;
short gOtherDiscCursorRow;
short gGakkuriPercent;
short gOpeningNode;
short gNextCursorColumn;
short gNextCursorRow;
short gHarumagePercent;
short gTokimekiPercent;
short gCurrentDisc;
short gDokudenpaCount;

static inline void CopyCellScreenPos(int x, int y)
{
    int level = y / 3;
    int cell = x + y % 3 * 8;

    if (gCaptionMoving == 0) {
        gCaptionX = gGridCells[level][cell].screenX;
        gCaptionY = gGridCells[level][cell].screenY;
    }
}

short gColumnLeftNoTurn[8] = { -1, -1, -1, 0, 3, -1, -1, 4 };
short gColumnRightNoTurn[8] = { 3, -1, -1, 4, 7, -1, -1, -1 };
short gColumnOrder[8] = { 0, -2, -1, 1, 2, 4, 5, 3 };
short gColumnRightOf[8] = { 3, 2, 0, 4, 7, 6, 1, 5 };
short gColumnLeftOf[8] = { 2, 6, 1, 0, 3, 7, 5, 4 };

DVECTOR gJumpToSpritePositions[7] = {
    { 229, 100 },
    { 253, 100 },
    { 242, 75 },
    { 242, 132 },
    { 233, 92 },
    { 0, 104 },
    { 229, 64 },
};
short gJumpToSlideStep;

void CopyCellNodeText(level, cell)
int level, cell;
{
    int i;
    int nodeIndex = gGridCells[level][cell].nodeIndex;

    if (nodeIndex >= 0) {
        strncpy(gNextCaptionText, gNodes[nodeIndex].name, 6);
        gNextCaptionText[6] = '\0';
        for (i = 0; i < 4; i++) {
            strncpy(gCaptionTitles[i],
                    gTitles[gNodes[gGridCells[level][cell].nodeIndex].titles[i]],
                    15);
            gCaptionTitles[i][15] = '\0';
        }
    } else {
        gCursorOnNode = 0;
        strcpy(gNextCaptionText, "Unknown");
        for (i = 0; i < 4; i++) {
            strcpy(gCaptionTitles[i], "");
        }
    }
}

enum CursorSnapMode {
    CURSOR_SNAP_ANY,
    CURSOR_SNAP_SAME_COLUMN,
    CURSOR_SNAP_NO_UP,
    CURSOR_SNAP_NO_DOWN,
    CURSOR_SNAP_NO_RIGHT,
    CURSOR_SNAP_NO_LEFT
};

/* A -1 entry looked up again reads the halfword before its table; for gColumnRightNoTurn that is gColumnLeftNoTurn[7]. */
int SnapNextCursorToNode(int columnMode, int rowMode)
{
    int relativeColumn;
    int candidate;
    int x;
    int y;
    unsigned short ySnapshot;
    int row;

    relativeColumn = gNextCursorColumn - gGridHeading;
    if (relativeColumn < 0) {
        relativeColumn += 8;
    }
    ySnapshot = gNextCursorRow;
    row = (short)((short)ySnapshot % 3);
    x = gNextCursorColumn;
    if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
        int level, cell;
        y = (short)ySnapshot + 1;
        level = y / 3;
        cell = x + (y % 3) * 8;
        if (gGridCells[level][cell].nodeIndex >= 0) {
            gNextCursorRow = ySnapshot + 1;
            return 1;
        }
    }
    if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
        int level, cell;
        y = gNextCursorRow - 1;
        level = y / 3;
        cell = x + (y % 3) * 8;
        if (gGridCells[level][cell].nodeIndex >= 0) {
            gNextCursorRow--;
            return 1;
        }
    }
    if (row <= 0 && rowMode != CURSOR_SNAP_NO_UP) {
        int level, cell;
        y = gNextCursorRow + 2;
        level = y / 3;
        cell = x + (y % 3) * 8;
        if (gGridCells[level][cell].nodeIndex >= 0) {
            gNextCursorRow += 2;
            return 1;
        }
    }
    if (row >= 2 && rowMode != CURSOR_SNAP_NO_DOWN) {
        int level, cell;
        y = gNextCursorRow - 2;
        level = y / 3;
        cell = x + (y % 3) * 8;
        if (gGridCells[level][cell].nodeIndex >= 0) {
            gNextCursorRow -= 2;
            return 1;
        }
    }
    if (columnMode == CURSOR_SNAP_SAME_COLUMN) {
        return 0;
    }

    candidate = gColumnRightNoTurn[relativeColumn];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_RIGHT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }

    candidate = gColumnLeftNoTurn[relativeColumn];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_LEFT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }

    candidate = gColumnRightNoTurn[gColumnRightNoTurn[relativeColumn]];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_RIGHT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }

    candidate = gColumnLeftNoTurn[gColumnLeftNoTurn[relativeColumn]];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_LEFT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }

    candidate = gColumnRightNoTurn[gColumnRightNoTurn[gColumnRightNoTurn[relativeColumn]]];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_RIGHT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }

    candidate = gColumnLeftNoTurn[gColumnLeftNoTurn[gColumnLeftNoTurn[relativeColumn]]];
    if (candidate >= 0 && columnMode != CURSOR_SNAP_NO_LEFT) {
        short ySnapshot;
        x = (candidate + gGridHeading) % 8;
        ySnapshot = gNextCursorRow;
        y = ySnapshot;
        {
            int level = y / 3;
            int cell = x + (y - level * 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                return 1;
            }
        }
        if (row < 2 && rowMode != CURSOR_SNAP_NO_UP) {
            int level, cell;
            y++;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow = ySnapshot + 1;
                return 1;
            }
        }
        if (row > 0 && rowMode != CURSOR_SNAP_NO_DOWN) {
            int level, cell;
            y = gNextCursorRow - 1;
            level = y / 3;
            cell = x + (y % 3) * 8;
            if (gGridCells[level][cell].nodeIndex >= 0) {
                gNextCursorColumn = x;
                gNextCursorRow--;
                return 1;
            }
        }
    }
    return 0;
}

short gPreviousCursorRow;
short gCursorOnNode;

int MoveGridCursor(int dx, int dy)
{
    int oldX = gCursorColumn;
    int columnOffset = gGridHeading;
    int relativeX = oldX - columnOffset;
    int x;
    int y;
    int level;
    int cell;

    gCursorOnNode = 1;
    gPreviousCursorRow = gCursorRow;
    if (relativeX < 0) {
        relativeX += 8;
    }
    if (dx > 0) {
        if (relativeX == 0) {
            gPadEvent = PAD_EVENT_L1;
            return 1;
        }
        relativeX = gColumnLeftOf[relativeX] + columnOffset;
        if (relativeX >= 8) {
            relativeX -= 8;
        }
        x = relativeX;
    } else if (dx < 0) {
        if (relativeX == 7) {
            gPadEvent = PAD_EVENT_R1;
            return 1;
        }
        relativeX = gColumnRightOf[relativeX] + columnOffset;
        if (relativeX >= 8) {
            relativeX -= 8;
        }
        x = relativeX;
    } else {
        x = oldX;
    }

    if (dy < 0) {
        int row = gCursorRow;
        int rowForStore = (unsigned short)gCursorRow;
        if (row > 0 && (row + dy) % 3 == 2) {
            gNextCursorRow = rowForStore + dy;
            gNextCursorColumn = x;
            gPadEvent = PAD_EVENT_PREVIOUS_LEVEL;
            return 1;
        }
    } else if (dy > 0) {
        int row = gCursorRow;
        int rowForStore = (unsigned short)gCursorRow;
        if ((row + dy) % 3 == 0) {
            gNextCursorRow = rowForStore + dy;
            gNextCursorColumn = x;
            gPadEvent = PAD_EVENT_NEXT_LEVEL;
            return 1;
        }
    }
    y = gCursorRow + dy;
    if (x < 0) {
        x = 7;
    } else if (x >= 8) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    level = y / 3;
    cell = x + (y - level * 3) * 8;
    if (gGridCells[level][cell].nodeIndex < 0) {
        relativeX = x - gGridHeading;
        if (relativeX < 0) {
            relativeX += 8;
        }
        if (dy > 0 && dx == 0) {
            gNextCursorColumn = x;
            gNextCursorRow = y;
            if (SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_DOWN) == 0) {
                gNextCursorRow = y + 3;
                gNextCursorColumn = x;
                gPadEvent = PAD_EVENT_NEXT_LEVEL;
                return 1;
            }
            x = gNextCursorColumn;
            y = gNextCursorRow;
        } else if (dy < 0 && dx == 0) {
            gNextCursorColumn = x;
            gNextCursorRow = y;
            if (SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_UP) == 0) {
                gNextCursorRow = y - 3;
                gNextCursorColumn = x;
                gPadEvent = PAD_EVENT_PREVIOUS_LEVEL;
                return 1;
            }
            x = gNextCursorColumn;
            y = gNextCursorRow;
        } else {
            gNextCursorColumn = x;
            gNextCursorRow = y;
            if (SnapNextCursorToNode(CURSOR_SNAP_SAME_COLUMN, CURSOR_SNAP_ANY) != 0) {
                x = gNextCursorColumn;
                y = gNextCursorRow;
            } else if (dx < 0) {
                if (gColumnOrder[relativeX] < 3) {
                    relativeX = gColumnRightOf[relativeX] + gGridHeading;
                    if (relativeX >= 8) {
                        relativeX -= 8;
                    }
                    level = y / 3;
                    x = relativeX;
                    cell = x + (y - level * 3) * 8;
                    if (gGridCells[level][cell].nodeIndex < 0) {
                        gNextCursorColumn = x;
                        gNextCursorRow = y;
                        if (SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_RIGHT) == 0) {
                            gPadEvent = PAD_EVENT_R1;
                            return 1;
                        }
                        x = gNextCursorColumn;
                        y = gNextCursorRow;
                    }
                } else {
                    gPadEvent = PAD_EVENT_R1;
                    return 1;
                }
            } else {
                if (gColumnOrder[relativeX] > 0) {
                    relativeX = gColumnLeftOf[relativeX] + gGridHeading;
                    if (relativeX >= 8) {
                        relativeX -= 8;
                    }
                    level = y / 3;
                    x = relativeX;
                    cell = x + (y - level * 3) * 8;
                    if (gGridCells[level][cell].nodeIndex < 0) {
                        gNextCursorColumn = x;
                        gNextCursorRow = y;
                        if (SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_LEFT) == 0) {
                            gPadEvent = PAD_EVENT_L1;
                            return 1;
                        }
                        x = gNextCursorColumn;
                        y = gNextCursorRow;
                    }
                } else {
                    gPadEvent = PAD_EVENT_L1;
                    return 1;
                }
            }
        }
        level = y / 3;
        cell = x + (y - level * 3) * 8;
    }
    if (gCursorColumn != x || gCursorRow != y) {
        gCaptionTargetX = gGridCells[level][cell].screenX;
        gCaptionTargetY = gGridCells[level][cell].screenY;
        CopyCellNodeText(level, cell);
        UpdateCaption(1);
        gCursorColumn = x;
        gCursorRow = y;
        gCursorLevel = level;
        gCursorCell = cell;
    } else {
        UpdateCaption(0);
    }
    return 0;
}

void UpdateCursorDuringMove(int commit)
{
    int row;
    int column;
    int level;
    int cellIndex;

    row = gCursorRow;
    column = gCursorColumn;
    level = row / 3;
    cellIndex = column + (row - gMoveStartLevel * 3) * 8;
    DrawCaptionDuringMove(gGridCells[level][cellIndex].screenX, gGridCells[level][cellIndex].screenY, commit);
    if (commit != 0) {
        row = gNextCursorRow;
        column = gNextCursorColumn;
        gCursorOnNode = 1;
        level = row / 3;
        cellIndex = column + (row % 3) * 8;
        if (gGridCells[level][cellIndex].nodeIndex < 0) {
            if (SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_ANY) == 0) {
                gCursorOnNode = 0;
            } else {
                column = gNextCursorColumn;
                row = gNextCursorRow;
            }
        }
        gCursorColumn = column;
        gCursorRow = row;
        level = row / 3;
        cellIndex = column + (row % 3) * 8;
        gCursorLevel = level;
        gCursorCell = cellIndex;
        gCaptionTargetX = gGridCells[level][cellIndex].screenX;
        gCaptionTargetY = gGridCells[level][cellIndex].screenY;
        CopyCellNodeText(level, cellIndex);
    }
}

void HandleGridInput(void)
{
    int moved;
    int dx;
    int dy;

    if (gPadEvent != PAD_EVENT_NONE) {
        gIdleFrames = 0;
    }
    moved = 0;
retry:
    switch (gPadEvent) {
    case PAD_EVENT_NONE:
        UpdateCaption(0);
        gIdleFrames++;
        return;
    case PAD_EVENT_UP: case PAD_EVENT_UP_LEFT: case PAD_EVENT_UP_RIGHT: case PAD_EVENT_LEFT:
    case PAD_EVENT_RIGHT: case PAD_EVENT_DOWN: case PAD_EVENT_DOWN_LEFT: case PAD_EVENT_DOWN_RIGHT:
        moved = 1;
        switch ((short)(gPadEvent - 1)) {
        case PAD_EVENT_UP - 1: dx = 0;  dy = 1;  break;
        case PAD_EVENT_UP_LEFT - 1: dx = 1;  dy = 1;  break;
        case PAD_EVENT_UP_RIGHT - 1: dx = -1; dy = 1;  break;
        case PAD_EVENT_LEFT - 1: dx = 1;  dy = 0;  break;
        case PAD_EVENT_RIGHT - 1: dx = -1; dy = 0;  break;
        case PAD_EVENT_DOWN - 1: dx = 0;  dy = -1; break;
        case PAD_EVENT_DOWN_LEFT - 1: dx = 1;  dy = -1; break;
        case PAD_EVENT_DOWN_RIGHT - 1: dx = -1; dy = -1; break;
        }
        if (!MoveGridCursor(dx, dy)) {
            return;
        }
        goto retry;
    case PAD_EVENT_R2:
        switch (gGridMenuState) {
        case GRID_MENU_REST:
            if (gTiltToggle == 0) {
                gPadEvent = PAD_EVENT_RIGHT_STICK_UP;
                gTiltToggle = 1;
            } else {
                gPadEvent = PAD_EVENT_RIGHT_STICK_DOWN;
                gTiltToggle = 0;
            }
            break;
        case GRID_MENU_TILTED_DOWN:
            gPadEvent = PAD_EVENT_RIGHT_STICK_UP;
            break;
        case GRID_MENU_TILTED_UP:
            gPadEvent = PAD_EVENT_RIGHT_STICK_DOWN;
            break;
        }
        goto retry;
    case PAD_EVENT_RIGHT_STICK_UP:
        switch (gGridMenuState) {
        case GRID_MENU_REST:
            gGridMenuReturnState = gGridMenuState;
            gGridMenuState = GRID_MENU_TILT_UP;
            break;
        case GRID_MENU_TILTED_DOWN:
            gGridMenuReturnState = gGridMenuState;
            gGridMenuState = GRID_MENU_UNTILT_FROM_DOWN;
            break;
        }
        UpdateCaption(0);
        return;
    case PAD_EVENT_RIGHT_STICK_DOWN:
        switch (gGridMenuState) {
        case GRID_MENU_REST:
            gGridMenuReturnState = gGridMenuState;
            gGridMenuState = GRID_MENU_TILT_DOWN;
            break;
        case GRID_MENU_TILTED_UP:
            gGridMenuReturnState = gGridMenuState;
            gGridMenuState = GRID_MENU_UNTILT_FROM_UP;
            break;
        }
        UpdateCaption(0);
        return;
    case PAD_EVENT_L1:
        gClipLoadRequested = LoadColumnTurnClips(0);
        UpdateCaption(0);
        gGridMenuReturnState = gGridMenuState;
        gGridMenuState = GRID_MENU_TURN_LEFT;
        gNextCursorColumn = gCursorColumn + 1;
        gNextCursorRow = gCursorRow;
        if (gNextCursorColumn >= 8) {
            gNextCursorColumn = gCursorColumn - 7;
        }
        return;
    case PAD_EVENT_R1:
        gClipLoadRequested = LoadColumnTurnClips(1);
        UpdateCaption(0);
        gGridMenuReturnState = gGridMenuState;
        gGridMenuState = GRID_MENU_TURN_RIGHT;
        gNextCursorColumn = gCursorColumn - 1;
        gNextCursorRow = gCursorRow;
        if (gNextCursorColumn < 0) {
            gNextCursorColumn = gCursorColumn + 7;
        }
        return;
    case PAD_EVENT_NEXT_LEVEL:
        UpdateCaption(0);
        if (gGridLevel < GetGridLevelCount() - 1) {
            gClipLoadRequested = LoadLevelMoveClip(0);
            gGridMenuReturnState = gGridMenuState;
            if (!moved) {
                gNextCursorColumn = gCursorColumn;
                gNextCursorRow = gCursorRow + 3;
            }
            switch (gGridMenuState) {
            case GRID_MENU_REST: gGridMenuState = GRID_MENU_LEVEL_UP; break;
            case GRID_MENU_TILTED_UP: gGridMenuState = GRID_MENU_UNTILT_UP_LEVEL_UP; break;
            case GRID_MENU_TILTED_DOWN: gGridMenuState = GRID_MENU_UNTILT_DOWN_LEVEL_UP; break;
            }
        }
        return;
    case PAD_EVENT_PREVIOUS_LEVEL:
        UpdateCaption(0);
        if (gGridLevel > 0) {
            gClipLoadRequested = LoadLevelMoveClip(1);
            gGridMenuReturnState = gGridMenuState;
            if (!moved) {
                gNextCursorColumn = gCursorColumn;
                gNextCursorRow = gCursorRow - 3;
            }
            switch (gGridMenuState) {
            case GRID_MENU_REST: gGridMenuState = GRID_MENU_LEVEL_DOWN; break;
            case GRID_MENU_TILTED_UP: gGridMenuState = GRID_MENU_UNTILT_UP_LEVEL_DOWN; break;
            case GRID_MENU_TILTED_DOWN: gGridMenuState = GRID_MENU_UNTILT_DOWN_LEVEL_DOWN; break;
            }
        }
        return;
    case PAD_EVENT_SQUARE:
        gShowAllTitles = ~gShowAllTitles;
        UpdateCaption(0);
        return;
    case PAD_EVENT_CIRCLE:
        gOpenClipSet = PickCursorClipSet();
        if (gOpenClipSet >= 0) {
            PlaySound(0);
            gClipLoadRequested = LoadNodeOpenClips(gOpenClipSet);
            gGridMenuReturnState = gGridMenuState;
            switch (gGridMenuState) {
            case GRID_MENU_REST: gGridMenuState = GRID_MENU_OPEN_NODE; break;
            case GRID_MENU_TILTED_UP: gGridMenuState = GRID_MENU_UNTILT_UP_OPEN_NODE; break;
            case GRID_MENU_TILTED_DOWN: gGridMenuState = GRID_MENU_UNTILT_DOWN_OPEN_NODE; break;
            }
        } else if (gHarumagePercent < 100) {
            gHarumagePercent++;
        }
        UpdateCaption(0);
        return;
    case PAD_EVENT_TRIANGLE:
        PlaySound(7);
        gClipLoadRequested = LoadSystemMenuClip();
        gGridMenuReturnState = gGridMenuState;
        switch (gGridMenuState) {
        case GRID_MENU_REST: gGridMenuState = GRID_MENU_SYSTEM_MENU; break;
        case GRID_MENU_TILTED_UP: gGridMenuState = GRID_MENU_UNTILT_UP_SYSTEM_MENU; break;
        case GRID_MENU_TILTED_DOWN: gGridMenuState = GRID_MENU_UNTILT_DOWN_SYSTEM_MENU; break;
        }
        UpdateCaption(0);
        return;
    case PAD_EVENT_L2:
        gGridMenuReturnState = gGridMenuState;
        gGridMenuState = GRID_MENU_JUMP_TO;
        UpdateCaption(0);
        return;
    default:
        UpdateCaption(0);
        return;
    }
}

void InitJumpToPanel(void)
{
    int tens;
    int ones;

    gJumpToSlideStep = 0;
    tens = (gGridLevel + 1) / 10;
    if (tens < 8) {
        GsSPRITE *digits = gJumpToSprites;
        digits[JUMP_TO_SPRITE_TENS].u = tens * 24 + 48;
        digits[JUMP_TO_SPRITE_TENS].v = 24;
    } else {
        GsSPRITE *digits = gJumpToSprites;
        digits[JUMP_TO_SPRITE_TENS].u = (tens - 8) * 24 + 48;
        digits[JUMP_TO_SPRITE_TENS].v = 56;
    }
    ones = (gGridLevel + 1) % 10;
    if (ones < 8) {
        GsSPRITE *digits = gJumpToSprites;
        digits[JUMP_TO_SPRITE_ONES].u = ones * 24 + 48;
        digits[JUMP_TO_SPRITE_ONES].v = 24;
    } else {
        GsSPRITE *digits = gJumpToSprites;
        digits[JUMP_TO_SPRITE_ONES].u = (ones - 8) * 24 + 48;
        digits[JUMP_TO_SPRITE_ONES].v = 56;
    }
}

void SlideJumpToSpritesIn(void)
{
    enum { SPRITE_COUNT = 7 };
    int horizontalIndex;
    int spriteIndex;
    int x, y;
    int slideStep;
    int yOffset;
    DVECTOR *positions, *horizontalPosition;

    slideStep = gJumpToSlideStep;
    if (slideStep < 9) {
        spriteIndex = 0;
        horizontalIndex = JUMP_TO_SPRITE_BAR;
        yOffset = slideStep * 22 - 176;
        positions = gJumpToSpritePositions;
        horizontalPosition = positions + JUMP_TO_SPRITE_BAR;
        for (; spriteIndex < SPRITE_COUNT; spriteIndex++) {
            if (spriteIndex != horizontalIndex) {
                x = positions[spriteIndex].vx;
                y = positions[spriteIndex].vy - yOffset;
            } else {
                x = positions[JUMP_TO_SPRITE_BAR].vx - 236 + slideStep * 236 / 8;
                y = horizontalPosition->vy;
            }
            gJumpToSprites[spriteIndex].x = x;
            gJumpToSprites[spriteIndex].y = y;
        }
        gJumpToSlideStep++;
    }
}

void SlideJumpToSpritesOut(void)
{
    enum { SPRITE_COUNT = 7 };
    int horizontalIndex;
    int spriteIndex;
    int x, y;
    int slideStep;
    int yOffset;
    DVECTOR *positions, *horizontalPosition;

    if (gJumpToSlideStep > 8) {
        gJumpToSlideStep = 8;
    }
    slideStep = gJumpToSlideStep;
    if (slideStep >= 0) {
        spriteIndex = 0;
        horizontalIndex = JUMP_TO_SPRITE_BAR;
        yOffset = slideStep * 22 - 176;
        positions = gJumpToSpritePositions;
        horizontalPosition = positions + JUMP_TO_SPRITE_BAR;
        for (; spriteIndex < SPRITE_COUNT; spriteIndex++) {
            if (spriteIndex != horizontalIndex) {
                x = positions[spriteIndex].vx;
                y = positions[spriteIndex].vy - yOffset;
            } else {
                x = positions[JUMP_TO_SPRITE_BAR].vx - 236 + slideStep * 236 / 8;
                y = horizontalPosition->vy;
            }
            gJumpToSprites[spriteIndex].x = x;
            gJumpToSprites[spriteIndex].y = y;
        }
        gJumpToSlideStep--;
    }
}


void SetJumpToDigits(int level)
{
    int digit;

    digit = (level + 1) / 10;
    if (digit < 8) {
        GsSPRITE *sprites = gJumpToSprites;
        sprites[JUMP_TO_SPRITE_TENS].u = 48 + 24 * digit;
        sprites[JUMP_TO_SPRITE_TENS].v = 24;
    } else {
        GsSPRITE *sprites = gJumpToSprites;
        sprites[JUMP_TO_SPRITE_TENS].u = 48 + 24 * (digit - 8);
        sprites[JUMP_TO_SPRITE_TENS].v = 56;
    }
    digit = (level + 1) % 10;
    if (digit < 8) {
        GsSPRITE *sprites = gJumpToSprites;
        sprites[JUMP_TO_SPRITE_ONES].u = 48 + 24 * digit;
        sprites[JUMP_TO_SPRITE_ONES].v = 24;
    } else {
        GsSPRITE *sprites = gJumpToSprites;
        sprites[JUMP_TO_SPRITE_ONES].u = 48 + 24 * (digit - 8);
        sprites[JUMP_TO_SPRITE_ONES].v = 56;
    }
}

void DrawJumpToSprites(void)
{
    int i;
    int visible;
    int levelCount;
    int level;
    GsSPRITE *sprites = gJumpToSprites;

    for (i = 0; i < 7; i++) {
        visible = 1;
        switch (i) {
        case JUMP_TO_SPRITE_UP_ARROW:
            level = gJumpToLevel;
            if (level != gGridLevel + 5) {
                levelCount = 13;
                level++;
                if (gCurrentDisc == 0) {
                    levelCount = 22;
                }
                if (level != levelCount) {
                    break;
                }
            }
            if (sprites[JUMP_TO_SPRITE_UP_ARROW].cx == 0) {
                visible = 0;
            }
            break;
        case JUMP_TO_SPRITE_DOWN_ARROW:
            if ((gJumpToLevel == gGridLevel - 5 || gJumpToLevel == 0) && sprites[JUMP_TO_SPRITE_DOWN_ARROW].cx == 0) {
                visible = 0;
            }
            break;
        }
        if (visible) {
            GsSortFastSprite(&sprites[i], &gSubOt[gActiveBuffer], 12);
        }
    }
}

inline void RestoreGridCursor(void)
{
    short unused[4]; /* 8 bytes nothing reads */
    int x = gCursorColumn;
    int level = (short)(gCursorRow / 3);
    int cell = x + (short)(gCursorRow % 3) * 8;

    gTokimekiPercent = 0;
    gDokudenpaCount = 0;
    gHarumagePercent = 0;
    gGakkuriPercent = 0;
    gCursorLevel = level;
    gCursorCell = cell;
    BuildNodeGrid();
    ResetGridSceneToCell(level, cell);
}

int RunGridMenu(void)
{
    RECT exitDispRect;
    RECT idleDispRect;
    RECT nodeTextRect;
    short endRollOrWord;
    short cell;
    short level;
    int captionPlaced;
    int exitCode;
    int nextState;
    short lineIndex;
    short playResult;

    captionPlaced = 0;
    PlaceCellCaption(0, 0, gCursorRow / 3, gCursorColumn + gCursorRow % 3 * 8);
    gGridMenuState = GRID_MENU_ENTER;
    gTiltToggle = 0;
    gGridMenuStep = 0;
    gSkipInputFrames = 0;
    gShowClipFrame = 1;
    gIdleFrames = 0;
    gOpeningNode = 0;
    PlayMusic();
    for (;;) {
        switch (gGridMenuState) {
        case GRID_MENU_ENTER:
            if (PlayGridEntryClip() == 0 && gGridBusy == 0) {
                gGridMenuState = GRID_MENU_REST;
                gShowClipFrame = 1;
                CopyCellScreenPos(gCursorColumn, gCursorRow);
            }
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            EndFrame();
            break;

        case GRID_MENU_REST:
        case GRID_MENU_TILTED_UP:
        case GRID_MENU_TILTED_DOWN:
            ReadPadEvent();
            if (gCaptionMoving == 0 && captionPlaced == 0) {
                int y = gCursorRow;

                gCaptionX = gGridCells[y / 3][gCursorColumn + y % 3 * 8].screenX;
                gCaptionY = gGridCells[y / 3][gCursorColumn + y % 3 * 8].screenY;
                captionPlaced = 1;
            }
            if (gIdleFrames >= 100) {
                UpdateIdleClips(0);
            } else {
                UpdateIdleClips(1);
            }
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            if (gCaptionMoving != 0) {
                UpdateCaption(0);
            } else if (gSkipInputFrames != 0) {
                gSkipInputFrames--;
            } else {
                HandleGridInput();
            }
            EndFrame();
            if (gIdleFrames >= 500 && AreIdleClipReadsDone() != 0) {
                CopyCellScreenPos(gCursorColumn, gCursorRow);
                gGridMenuReturnState = gGridMenuState;
                gGridMenuState = GRID_MENU_IDLE_MEDIA;
                gSceneCommand = SCENE_CMD_QUAD_EXPAND;
                gIdleFrames = 0;
                if (gDokudenpaCount < 9) {
                    gDokudenpaCount++;
                }
            }
            break;

        case GRID_MENU_NO_SHARED_NODE:
            ReadPadEvent();
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            DrawNoSharedNodePanel();
            EndFrame();
            if (gPadEvent == PAD_EVENT_CIRCLE) {
                gPadEvent = PAD_EVENT_NONE;
                gGridMenuState = gGridMenuReturnState;
            }
            break;

        case GRID_MENU_IDLE_MEDIA:
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            UpdateCaption(0);
            EndFrame();
            if (gGridBusy == 0) {
                setRECT(&idleDispRect, 0, 0, 320, 480);
                ClearImage(&idleDispRect, 0, 0, 0);
                FreeIdleClips();
                FadeOutMusic();
                PlaySound(30);
                PlayIdleMedia();
                gGridMenuState = gGridMenuReturnState;
                FadeInMusic();
            }
            break;

        case GRID_MENU_OPEN_NODE:
            if (gGridMenuStep == 0) {
                if (gClipLoadRequested == 0) {
                    gClipLoadRequested = LoadNodeOpenClips(gOpenClipSet);
                    UpdateIdleClips(1);
                } else if (PollNodeOpenClips() == 0) {
                    UpdateIdleClips(1);
                } else if (PlayNodeOpenClips() >= 0) {
                    int cellIndex;

                    gOpeningNode = 1;
                    gSceneCommand = SCENE_CMD_CELL_FLIGHT;
                    gGridMenuStep++;
                    cellIndex = gCursorColumn + (gCursorRow - gGridLevel * 3) * 8;
                    gCursorCell = cellIndex;
                    gCursorLevel = gCursorRow / 3;
                }
            } else if (PlayNodeOpenClips() == 0) {
                gGridMenuStep = 0;
                gGridMenuState = gGridMenuReturnState;
            }
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            UpdateCaption(0);
            EndFrame();
            if (gGridMenuState == gGridMenuReturnState && gLoadedOpenClipSet < OPEN_CLIP_REFUSE_SPARK) {
                if (gTokimekiPercent < 100) {
                    gTokimekiPercent++;
                }
                setRECT(&nodeTextRect, 320, 0, 320, 480);
                level = gCursorRow / 3;
                cell = gCursorColumn + gCursorRow % 3 * 8;
                ClearImage(&nodeTextRect, 0, 0, 0);
                gCellFlying = 0;
                FadeOutMusic();
                if (IsCursorNodeMedia() != 0) {
                    UploadTextToVram(gNodes[gGridCells[level][cell].nodeIndex].name, 320, 120, TEXT_UPLOAD_SMALL, 1);
                    for (lineIndex = 0; lineIndex < 3; lineIndex++) {
                        if (gNodes[gGridCells[level][cell].nodeIndex].words[lineIndex] >= 0) {
                            UploadTextToVram(gWords[gNodes[gGridCells[level][cell].nodeIndex].words[lineIndex]], 320, lineIndex * 16 + 72, TEXT_UPLOAD_SMALL, 1);
                        } else {
                            UploadTextToVram("", 320, lineIndex * 16 + 72, TEXT_UPLOAD_SMALL, 1);
                        }
                    }
                    FreeIdleClips();
                    InitTileGrid("", 351);
                    endRollOrWord = (gNodes[gGridCells[level][cell].nodeIndex].flags >> 27) & 1;
                    playResult = RunMediaPlayer(gNodes[gGridCells[level][cell].nodeIndex].track, &endRollOrWord, gNodes[gGridCells[level][cell].nodeIndex].imageParts);
                    gSkipInputFrames = 2;
                    gOpeningNode = 0;
                    FreeTileGrid();
                    ReadPadEvent();
                    switch (playResult) {
                    case MEDIA_RESULT_END_ROLL:
                        if (gEndRollCount < 9999) {
                            gEndRollCount++;
                        }
                        MarkNodeOpened(gGridCells[level][cell].nodeIndex, 1);
                        if (gGridCells[level][cell].modelType < 8) {
                            gGridCells[level][cell].modelType += 8;
                            RebuildCellModel(level, cell);
                        }
                        exitCode = RunContinuePrompt();
                        if (exitCode != GRID_MENU_EXIT_START) {
                            int x;
                            int y;

                            x = gCursorColumn;
                            y = gCursorRow;
                            gCursorColumn = gOtherDiscCursorColumn;
                            gCursorRow = gOtherDiscCursorRow;
                            gOtherDiscCursorColumn = x;
                            gOtherDiscCursorRow = y;
                            if (gCurrentDisc != 0) {
                                gCurrentDisc = 0;
                            } else {
                                gCurrentDisc = 1;
                            }
                            gCursorColumn = 6;
                            gCursorRow = 10;
                            gOtherDiscCursorColumn = 5;
                            gOtherDiscCursorRow = 0;
                        }
                        goto exit;
                    case MEDIA_RESULT_COMPLETE:
                        MarkNodeOpened(gGridCells[level][cell].nodeIndex, 0);
                        if (gGridCells[level][cell].modelType < 8) {
                            gGridCells[level][cell].modelType += 8;
                            RebuildCellModel(level, cell);
                        }
                        break;
                    }
                    if (endRollOrWord != 0) {
                        if (FindNodeSharingWord(gGridCells[level][cell].nodeIndex, endRollOrWord - 1, &cell, &level) != 0) {
                            gGridMenuReturnState = gGridMenuState;
                            gGridMenuState = GRID_MENU_LINK_TRAVEL;
                            gSceneCommand = SCENE_CMD_TRAVEL_TO_CURSOR;
                            gCursorLevel = level;
                            gCursorCell = cell;
                            gLinkedNodeLevel = level;
                            gLinkedNodeCell = cell;
                            gCursorColumn = cell % 8;
                            gCursorRow = level * 3 + cell / 8;
                            gShowClipFrame = 0;
                        } else {
                            PlaySound(28);
                            gGridMenuReturnState = gGridMenuState;
                            gGridMenuState = GRID_MENU_NO_SHARED_NODE;
                            gOpeningNode = 0;
                        }
                    } else {
                        gOpeningNode = 0;
                    }
                } else {
                    int itemKind = (gNodes[gGridCells[level][cell].nodeIndex].flags >> 20) & 0xf;
                    switch (itemKind) {
                    case NODE_KIND_UPGRADE:
                        if (RunUpgradeScreen() != 0) {
                            MarkNodeOpened(gGridCells[level][cell].nodeIndex, 0);
                            gGridCells[level][cell].nodeIndex = -1;
                            RebuildCellModel(level, cell);
                            gUpgradesApplied++;
                            PlaceCellCaption(0, 0, level, cell);
                            CopyCellScreenPos(gCursorColumn, gCursorRow);
                        }
                        break;
                    case NODE_KIND_GATE:
                        RunGateScreen();
                        MarkNodeOpened(gGridCells[level][cell].nodeIndex, 0);
                        gGridCells[level][cell].nodeIndex = -1;
                        RebuildCellModel(level, cell);
                        PlaceCellCaption(0, 0, level, cell);
                        CopyCellScreenPos(gCursorColumn, gCursorRow);
                        break;
                    case NODE_KIND_P2:
                        if (strcmp("P2-01", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 2;
                        } else if (strcmp("P2-02", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 4;
                        } else if (strcmp("P2-03", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 0x10;
                        } else if (strcmp("P2-04", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 8;
                        } else if (strcmp("P2-05", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 0x20;
                        } else if (strcmp("P2-06", gNodes[gGridCells[level][cell].nodeIndex].name) == 0) {
                            gP2NodesOpened |= 1;
                        }
                        RunP2Screen();
                        MarkNodeOpened(gGridCells[level][cell].nodeIndex, 0);
                        gGridCells[level][cell].nodeIndex = -1;
                        RebuildCellModel(level, cell);
                        PlaceCellCaption(0, 0, level, cell);
                        CopyCellScreenPos(gCursorColumn, gCursorRow);
                        break;
                    }
                }
                FadeInMusic();
            } else if (gGakkuriPercent < 100) {
                gGakkuriPercent++;
            }
            break;

        case GRID_MENU_JUMP_TO:
            switch (gGridMenuStep) {
            case JUMP_TO_INIT:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                gCaptionTargetX = 138;
                gCaptionTargetY = 106;
                strcpy(gNextCaptionText, "Jump To");
                InitJumpToPanel();
                SetCaptionStyle(CAPTION_STYLE_PLAIN);
                UpdateCaption(1);
                EndFrame();
                gGridMenuStep++;
                gJumpToLevel = gGridLevel;
                break;
            case JUMP_TO_SLIDE_IN:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                SlideJumpToSpritesIn();
                DrawJumpToSprites();
                UpdateCaption(0);
                EndFrame();
                if (gCaptionMoving == 0) {
                    gGridMenuStep++;
                }
                break;
            case JUMP_TO_CHOOSE:
                ReadPadEvent();
                gJumpToSprites[JUMP_TO_SPRITE_UP_ARROW].cx = 0;
                gJumpToSprites[JUMP_TO_SPRITE_DOWN_ARROW].cx = 0;
                switch (gPadEvent) {
                case PAD_EVENT_UP:
                case PAD_EVENT_UP_LEFT:
                case PAD_EVENT_UP_RIGHT: {
                    short current = gJumpToLevel;
                    int next = current + 1;
                    int limit;

                    if (gCurrentDisc == 0) {
                        limit = 22;
                    } else {
                        limit = 13;
                    }
                    if (next < limit && next <= gGridLevel + 5) {
                        gJumpToLevel = current + 1;
                        SetJumpToDigits(gJumpToLevel);
                        gJumpToSprites[JUMP_TO_SPRITE_UP_ARROW].cx = 16;
                    }
                    break;
                }
                case PAD_EVENT_DOWN:
                case PAD_EVENT_DOWN_LEFT:
                case PAD_EVENT_DOWN_RIGHT:
                    if (gJumpToLevel - 1 >= 0 && gJumpToLevel - 1 >= gGridLevel - 5) {
                        gJumpToLevel--;
                        SetJumpToDigits(gJumpToLevel);
                        gJumpToSprites[JUMP_TO_SPRITE_DOWN_ARROW].cx = 16;
                    }
                    break;
                case PAD_EVENT_CIRCLE:
                    if (gJumpToLevel != gGridLevel) {
                        if (gJumpToLevel > gGridLevel) {
                            gClipLoadRequested = LoadLevelMoveClip(0);
                        } else {
                            gClipLoadRequested = LoadLevelMoveClip(1);
                        }
                        gJumpToLevelChanged = 1;
                    } else {
                        gJumpToLevelChanged = 0;
                    }
                    gGridMenuStep++;
                    gCaptionTargetX = gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenX;
                    gCaptionTargetY = gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenY;
                    CopyCellNodeText(gCursorRow / 3, gCursorColumn + gCursorRow % 3 * 8);
                    break;
                case PAD_EVENT_CROSS:
                    gGridMenuStep++;
                    gCaptionTargetX = gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenX;
                    gCaptionTargetY = gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenY;
                    gJumpToLevelChanged = 0;
                    CopyCellNodeText(gCursorRow / 3, gCursorColumn + gCursorRow % 3 * 8);
                    break;
                }
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                DrawJumpToSprites();
                if (gGridMenuStep != JUMP_TO_CHOOSE) {
                    SetCaptionStyle(CAPTION_STYLE_RESTORE_CELL);
                    UpdateCaption(1);
                } else {
                    UpdateCaption(0);
                }
                EndFrame();
                break;
            case JUMP_TO_SLIDE_OUT:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                SlideJumpToSpritesOut();
                DrawJumpToSprites();
                UpdateCaption(0);
                EndFrame();
                if (gCaptionMoving == 0) {
                    gGridMenuStep++;
                }
                break;
            case JUMP_TO_CHECK_TRAVEL:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                UpdateCaption(0);
                EndFrame();
                if (gJumpToLevelChanged != 0) {
                    gGridMenuStep++;
                } else {
                    gGridMenuStep = 0;
                    gGridMenuState = gGridMenuReturnState;
                }
                break;
            case JUMP_TO_LOAD_CLIP:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                UpdateCaption(0);
                EndFrame();
                if (gClipLoadRequested != 0) {
                    if (PollLevelMoveClip() != 0) {
                        gGridMenuStep++;
                    }
                } else {
                    if (gJumpToLevel > gGridLevel) {
                        gClipLoadRequested = LoadLevelMoveClip(0);
                    } else {
                        gClipLoadRequested = LoadLevelMoveClip(1);
                    }
                }
                break;
            case JUMP_TO_START_TRAVEL:
                if (PlayLevelMoveClip() >= 0) {
                    int nextLevel;
                    int nextCell;

                    gNextCursorRow = (gJumpToLevel - gGridLevel) * 3 + gCursorRow;
                    gNextCursorColumn = gCursorColumn;
                    nextLevel = (short)(gNextCursorRow / 3);
                    nextCell = gNextCursorColumn + (short)(gNextCursorRow % 3) * 8;
                    if (gGridCells[nextLevel][nextCell].nodeIndex < 0) {
                        SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_ANY);
                        nextLevel = (short)(gNextCursorRow / 3);
                        nextCell = gNextCursorColumn + (short)(gNextCursorRow % 3) * 8;
                    }
                    gSceneCommand = SCENE_CMD_LEVEL_TRAVEL;
                    gCursorLevel = nextLevel;
                    gCursorCell = nextCell;
                    gGridMenuStep++;
                }
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                UpdateCaption(0);
                EndFrame();
                break;
            case JUMP_TO_WAIT_TRAVEL:
                if (PlayLevelMoveClip() == 0) {
                    if (gGridBusy == 0) {
                        gGridMenuStep++;
                    }
                }
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                EndFrame();
                break;
            case JUMP_TO_FINISH: {
                short y = gNextCursorRow;
                short p = y / 3;
                short x = gNextCursorColumn;
                int c = x + (short)(y % 3) * 8;

                gCursorColumn = x;
                gCursorRow = y;
                PlaceCellCaption(gGridCells[p][c].screenX, gGridCells[p][c].screenY, p, c);
                CopyCellScreenPos(gCursorColumn, gCursorRow);
                gGridMenuStep = 0;
                gGridMenuState = gGridMenuReturnState;
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                UpdateCaption(0);
                EndFrame();
                break;
            }
            }
            break;

        case GRID_MENU_LINK_TRAVEL:
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            EndFrame();
            switch (gGridMenuStep) {
            case LINK_TRAVEL_MOVE:
                if (gGridBusy == 0) {
                    BeginFrame();
                    UpdateGridScene();
                    if (gShowClipFrame != 0) {
                        DrawClipFrame();
                    }
                    StepHorizonAndStreaks();
                    EndFrame();
                    PlaceCellCaption(gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenX,
                                  gGridCells[gCursorRow / 3][gCursorColumn + gCursorRow % 3 * 8].screenY,
                                  gCursorRow / 3, gCursorColumn + gCursorRow % 3 * 8);
                    CopyCellScreenPos(gCursorColumn, gCursorRow);
                    gSceneCommand = SCENE_CMD_FLASH_CLIP_FRAME;
                    gGridMenuStep++;
                    UpdateIdleClips(1);
                    gShowClipFrame = 1;
                }
                break;
            case LINK_TRAVEL_FLASH:
                if (gGridBusy == 0) {
                    gGridMenuStep = 0;
                    gGridMenuState = gGridMenuReturnState;
                }
                break;
            }
            break;

        case GRID_MENU_SYSTEM_MENU:
            switch (gGridMenuStep) {
            case SYSTEM_MENU_LOAD_CLIP:
                UpdateIdleClips(1);
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                CopyCellScreenPos(gCursorColumn, gCursorRow);
                UpdateCaption(0);
                EndFrame();
                if (gClipLoadRequested != 0) {
                    if (PollSystemMenuClip() != 0) {
                        gSceneCommand = SCENE_CMD_RING_BURST;
                        gGridMenuStep++;
                    }
                } else {
                    gClipLoadRequested = LoadSystemMenuClip();
                }
                break;
            case SYSTEM_MENU_WAIT_BURST:
                if (PlaySystemMenuClip() == 0 && gGridBusy == 0) {
                    gGridMenuStep++;
                }
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                CopyCellScreenPos(gCursorColumn, gCursorRow);
                UpdateCaption(0);
                EndFrame();
                break;
            case SYSTEM_MENU_START_RISE:
                gSceneCommand = SCENE_CMD_VIEW_TO_TOP;
                gGridMenuStep++;
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                EndFrame();
                break;
            case SYSTEM_MENU_WAIT_RISE:
                if (gGridBusy == 0) {
                    gGridMenuStep++;
                }
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                EndFrame();
                break;
            case SYSTEM_MENU_OPEN:
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                if (StepSystemMenuOpen() != 0) {
                    gGridMenuStep++;
                }
                EndFrame();
                break;
            case SYSTEM_MENU_RUN: {
                int selection;

                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                switch (UpdateSystemMenu()) {
                case MENU_RESULT_EXIT:
                    gGridMenuStep++;
                    EndFrame();
                    selection = 0;
                    break;
                case MENU_RESULT_LOAD:
                    selection = GRID_MENU_EXIT_LOAD;
                    break;
                case MENU_RESULT_CHANGE:
                    selection = GRID_MENU_EXIT_SITE_CHANGE;
                    break;
                default:
                    EndFrame();
                    selection = 0;
                    break;
                }
                switch (selection) {
                case GRID_MENU_EXIT_LOAD:
                    exitCode = GRID_MENU_EXIT_LOAD;
                    RestoreGridCursor();
                    goto exit;
                case GRID_MENU_EXIT_SITE_CHANGE: {
                    int x;
                    int y;
                    short otherX;
                    short otherY;

                    x = gCursorColumn;
                    y = gCursorRow;
                    otherX = gOtherDiscCursorColumn;
                    otherY = gOtherDiscCursorRow;
                    gCursorColumn = otherX;
                    gCursorRow = otherY;
                    gOtherDiscCursorColumn = x;
                    gOtherDiscCursorRow = y;
                    exitCode = GRID_MENU_EXIT_SITE_CHANGE;
                    if (gCurrentDisc != 0) {
                        gCurrentDisc = 0;
                    } else {
                        gCurrentDisc = 1;
                    }
                    RestoreGridCursor();
                    goto exit;
                }
                }
                break;
            }
            case SYSTEM_MENU_START_CLOSE:
                gSceneCommand = SCENE_CMD_VIEW_TO_FRONT;
                gGridMenuStep++;
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                StepSystemMenuClose();
                EndFrame();
                break;
            case SYSTEM_MENU_CLOSE:
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                if (StepSystemMenuClose() != 0) {
                    gGridMenuStep++;
                }
                EndFrame();
                break;
            case SYSTEM_MENU_WAIT_RETURN:
                if (gGridBusy == 0) {
                    gGridMenuStep = 0;
                    gGridMenuState = gGridMenuReturnState;
                }
                BeginFrame();
                UpdateGridScene();
                if (gShowClipFrame != 0) {
                    DrawClipFrame();
                }
                StepHorizonAndStreaks();
                EndFrame();
                break;
            }
            break;

        case GRID_MENU_TILT_UP:
            if (gGridMenuStep == 0) {
                gSceneCommand = SCENE_CMD_TILT_UP;
                gGridMenuStep++;
                PlaySound(29);
            } else if (gGridBusy == 0) {
                gGridMenuState = GRID_MENU_TILTED_UP;
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_UNTILT_FROM_UP:
            if (gGridMenuStep == 0) {
                gSceneCommand = SCENE_CMD_TILT_DOWN;
                gGridMenuStep++;
                PlaySound(29);
            } else if (gGridBusy == 0) {
                gGridMenuState = GRID_MENU_REST;
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_TILT_DOWN:
            if (gGridMenuStep == 0) {
                gSceneCommand = SCENE_CMD_TILT_DOWN;
                gGridMenuStep++;
                PlaySound(29);
            } else if (gGridBusy == 0) {
                gGridMenuState = GRID_MENU_TILTED_DOWN;
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_UNTILT_FROM_DOWN:
            if (gGridMenuStep == 0) {
                gSceneCommand = SCENE_CMD_TILT_UP;
                gGridMenuStep++;
                PlaySound(29);
            } else if (gGridBusy == 0) {
                gGridMenuState = GRID_MENU_REST;
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_TURN_LEFT:
        case 11:
        case 12:
        case GRID_MENU_TURN_RIGHT:
        case 14:
        case 15:
            nextState = 0;
            if (gGridMenuStep == 0) {
                if (gClipLoadRequested == 0) {
                    switch (gGridMenuState) {
                    case GRID_MENU_TURN_LEFT ... 12:
                        gClipLoadRequested = LoadColumnTurnClips(0);
                        break;
                    case GRID_MENU_TURN_RIGHT ... 15:
                        gClipLoadRequested = LoadColumnTurnClips(1);
                        break;
                    }
                    UpdateIdleClips(1);
                } else if (PollColumnTurnClips() == 0) {
                    UpdateIdleClips(1);
                } else if (PlayColumnTurnClips() >= 0) {
                    gGridMenuStep++;
                    gMoveStartLevel = gGridLevel;
                    switch (gGridMenuState) {
                    case GRID_MENU_TURN_LEFT ... 12:
                        gSceneCommand = SCENE_CMD_TURN_LEFT;
                        break;
                    case GRID_MENU_TURN_RIGHT ... 15:
                        gSceneCommand = SCENE_CMD_TURN_RIGHT;
                        break;
                    }
                    nextState = 1;
                    PlaySound(6);
                }
            } else if (PlayColumnTurnClips() == 0) {
                nextState = gGridMenuState;
                gGridMenuStep = 0;
                gGridMenuState = gGridMenuReturnState;
            } else {
                nextState = 1;
            }
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            if (nextState == 0) {
                UpdateCaption(0);
            } else {
                UpdateCursorDuringMove(nextState == 1 ? 0 : nextState);
            }
            EndFrame();
            if (gGridMenuState == gGridMenuReturnState) {
                gSceneCommand = SCENE_CMD_NONE;
            }
            break;

        case GRID_MENU_LEVEL_UP:
        case GRID_MENU_LEVEL_DOWN:
            nextState = 0;
            if (gGridMenuStep == 0) {
                if (gClipLoadRequested == 0) {
                    switch (gGridMenuState) {
                    case GRID_MENU_LEVEL_UP:
                        gClipLoadRequested = LoadLevelMoveClip(0);
                        break;
                    case GRID_MENU_LEVEL_DOWN:
                        gClipLoadRequested = LoadLevelMoveClip(1);
                        break;
                    }
                    UpdateIdleClips(1);
                } else if (PollLevelMoveClip() == 0) {
                    UpdateIdleClips(1);
                } else if (PlayLevelMoveClip() >= 0) {
                    gGridMenuStep++;
                    gMoveStartLevel = gGridLevel;
                    switch (gGridMenuState) {
                    case GRID_MENU_LEVEL_UP:
                        gSceneCommand = SCENE_CMD_LEVEL_UP;
                        PlaySound(4);
                        break;
                    case GRID_MENU_LEVEL_DOWN:
                        gSceneCommand = SCENE_CMD_LEVEL_DOWN;
                        PlaySound(5);
                        break;
                    }
                    nextState = 1;
                }
            } else if (PlayLevelMoveClip() == 0) {
                nextState = gGridMenuState;
                gGridMenuStep = 0;
                gGridMenuState = gGridMenuReturnState;
            } else {
                nextState = 1;
            }
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            if (nextState == 0) {
                UpdateCaption(0);
            } else {
                UpdateCursorDuringMove(nextState == 1 ? 0 : nextState);
            }
            EndFrame();
            if (gGridMenuState == gGridMenuReturnState) {
                gSceneCommand = SCENE_CMD_NONE;
            }
            break;

        case GRID_MENU_UNTILT_UP_LEVEL_UP:
        case GRID_MENU_UNTILT_UP_LEVEL_DOWN:
        case GRID_MENU_UNTILT_DOWN_LEVEL_UP:
        case GRID_MENU_UNTILT_DOWN_LEVEL_DOWN:
            if (gGridMenuStep == 0) {
                switch (gGridMenuState) {
                case GRID_MENU_UNTILT_UP_LEVEL_UP ... GRID_MENU_UNTILT_UP_LEVEL_DOWN:
                    gSceneCommand = SCENE_CMD_TILT_DOWN;
                    break;
                case GRID_MENU_UNTILT_DOWN_LEVEL_UP ... GRID_MENU_UNTILT_DOWN_LEVEL_DOWN:
                    gSceneCommand = SCENE_CMD_TILT_UP;
                    break;
                }
                gGridMenuReturnState = GRID_MENU_REST;
                gGridMenuStep++;
            } else if (gGridBusy == 0) {
                switch (gGridMenuState) {
                case GRID_MENU_UNTILT_UP_LEVEL_UP:
                    gGridMenuState = GRID_MENU_LEVEL_UP;
                    break;
                case GRID_MENU_UNTILT_UP_LEVEL_DOWN:
                    gGridMenuState = GRID_MENU_LEVEL_DOWN;
                    break;
                case GRID_MENU_UNTILT_DOWN_LEVEL_UP:
                    gGridMenuState = GRID_MENU_LEVEL_UP;
                    break;
                case GRID_MENU_UNTILT_DOWN_LEVEL_DOWN:
                    gGridMenuState = GRID_MENU_LEVEL_DOWN;
                    break;
                }
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
            }
            UpdateIdleClips(1);
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_UNTILT_UP_OPEN_NODE:
        case GRID_MENU_UNTILT_DOWN_OPEN_NODE:
            if (gGridMenuStep == 0) {
                switch (gGridMenuState) {
                case GRID_MENU_UNTILT_UP_OPEN_NODE:
                    gSceneCommand = SCENE_CMD_TILT_DOWN;
                    break;
                case GRID_MENU_UNTILT_DOWN_OPEN_NODE:
                    gSceneCommand = SCENE_CMD_TILT_UP;
                    break;
                }
                gGridMenuReturnState = GRID_MENU_REST;
                gGridMenuStep++;
            } else if (gGridBusy == 0) {
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
                gGridMenuState = GRID_MENU_OPEN_NODE;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;

        case GRID_MENU_UNTILT_UP_SYSTEM_MENU:
        case GRID_MENU_UNTILT_DOWN_SYSTEM_MENU:
            if (gGridMenuStep == 0) {
                switch (gGridMenuState) {
                case GRID_MENU_UNTILT_UP_SYSTEM_MENU:
                    gSceneCommand = SCENE_CMD_TILT_DOWN;
                    break;
                case GRID_MENU_UNTILT_DOWN_SYSTEM_MENU:
                    gSceneCommand = SCENE_CMD_TILT_UP;
                    break;
                }
                gGridMenuReturnState = GRID_MENU_REST;
                gGridMenuStep++;
            } else if (gGridBusy == 0) {
                gSceneCommand = SCENE_CMD_NONE;
                gGridMenuStep = 0;
                gGridMenuState = GRID_MENU_SYSTEM_MENU;
            }
            UpdateIdleClips(1);
            BeginFrame();
            UpdateGridScene();
            if (gShowClipFrame != 0) {
                DrawClipFrame();
            }
            StepHorizonAndStreaks();
            CopyCellScreenPos(gCursorColumn, gCursorRow);
            UpdateCaption(0);
            EndFrame();
            break;
        }
    }
exit:
    setRECT(&exitDispRect, 0, 0, 320, 480);
    ClearImage(&exitDispRect, 0, 0, 0);
    StopGridMusic();
    return exitCode;
}

void EndSceneFrame(void)
{
    EndFrame();
}

void BeginGridFrame(void)
{
    BeginFrame();
    UpdateGridScene();
    if (gShowClipFrame != 0) {
        DrawClipFrame();
    }
    StepHorizonAndStreaks();
}

int GetGridLevelCount(void)
{
    if (gCurrentDisc == 0) {
        return 22;
    }
    return 13;
}

int gLoadCount;

void InitGridState(void)
{
    short unused[4]; /* 8 bytes nothing reads */
    int x = 6;
    int y = 10;
    int level = y / 3;
    int cell = x + y % 3 * 8;
    gCursorColumn = x;
    gCursorRow = y;
    gOtherDiscCursorColumn = 5;
    gCursorLevel = level;
    gCurrentDisc = 0;
    gGatesOpened = 0;
    gP2NodesOpened = 0;
    gUpgradesApplied = 0;
    gEndRollCount = 0;
    gOtherDiscCursorRow = 0;
    gTokimekiPercent = 0;
    gDokudenpaCount = 0;
    gHarumagePercent = 0;
    gGakkuriPercent = 0;
    gCursorCell = cell;
    ResetGridSceneToCell(level, cell);
    gCursorOnNode = 1;
    gLoadCount = 0;
}
