#ifndef PAD_H
#define PAD_H

#include <sys/types.h>

/*
 * ReadPadEvent's code for this frame's new press: d-pad or left stick (UP..DOWN_RIGHT),
 * L1/R1 also from right stick left/right, TRIANGLE also from select. Only MoveGridCursor
 * writes NEXT_LEVEL and PREVIOUS_LEVEL.
 */
enum PadEvent {
    PAD_EVENT_NONE,
    PAD_EVENT_UP,
    PAD_EVENT_UP_LEFT,
    PAD_EVENT_UP_RIGHT,
    PAD_EVENT_LEFT,
    PAD_EVENT_RIGHT,
    PAD_EVENT_DOWN,
    PAD_EVENT_DOWN_LEFT,
    PAD_EVENT_DOWN_RIGHT,
    PAD_EVENT_R2,
    PAD_EVENT_RIGHT_STICK_UP,
    PAD_EVENT_RIGHT_STICK_DOWN,
    PAD_EVENT_L1,
    PAD_EVENT_R1,
    PAD_EVENT_L2,
    PAD_EVENT_NEXT_LEVEL,
    PAD_EVENT_PREVIOUS_LEVEL,
    PAD_EVENT_CIRCLE,
    PAD_EVENT_CROSS,
    PAD_EVENT_TRIANGLE,
    PAD_EVENT_START,
    PAD_EVENT_SQUARE
};
extern short gPadEvent;
enum SceneCommand {
    SCENE_CMD_NONE,
    SCENE_CMD_TILT_UP = 10,
    SCENE_CMD_TILT_DOWN,
    SCENE_CMD_TURN_LEFT,
    SCENE_CMD_TURN_RIGHT,
    SCENE_CMD_LEVEL_UP = 15,
    SCENE_CMD_LEVEL_DOWN,
    SCENE_CMD_TRAVEL_TO_CURSOR = 22,
    SCENE_CMD_LEVEL_TRAVEL,
    SCENE_CMD_VIEW_TO_TOP,
    SCENE_CMD_VIEW_TO_FRONT,
    SCENE_CMD_CELL_FLIGHT,
    SCENE_CMD_RING_BURST,
    SCENE_CMD_QUAD_EXPAND,
    SCENE_CMD_FLASH_CLIP_FRAME
};
extern short gSceneCommand;
/* Grid cursor storage defined in pad's .sbss; read and written by grid_menu and grid_scene. */
extern short gCursorCell;
extern short gCursorLevel;

void ReadPadDirection(void);
void ReadPadEvent(void);
void InitPads(void);

#endif
