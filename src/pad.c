/* Controller input: LIBPAD set-up, and the per-frame read of port 1's pad into one pad event a press. */
#include <sys/types.h>
#include <libetc.h>
#include <libpad.h>

#include "pad.h"

short gPadEvent;
short gSceneCommand;

short gLastPadEvent;
/*
 * LIBPAD's receive buffers, port 1's then port 2's. In each: [0] 0 when the pad answered; [1] its type
 * in the high nibble (4 digital, 7 analog); [2] and [3] the buttons as the high and low byte of
 * libetc.h's PAD masks, a bit clear while held; [4] [5] the right stick's X and Y, [6] [7] the left's,
 * 0x80 at rest.
 */
static u_char sPadBuffer[2][36];

void ReadPadDirection(void)
{
    unsigned int buttons = sPadBuffer[0][2];

    if (!(buttons & (PADLup >> 8))) {
        if (!(buttons & (PADLleft >> 8))) {
            gPadEvent = PAD_EVENT_UP_LEFT;
        } else if (!(buttons & (PADLright >> 8))) {
            gPadEvent = PAD_EVENT_UP_RIGHT;
        } else {
            gPadEvent = PAD_EVENT_UP;
        }
    } else if (!(buttons & (PADLdown >> 8))) {
        if (!(buttons & (PADLleft >> 8))) {
            gPadEvent = PAD_EVENT_DOWN_LEFT;
        } else if (!(buttons & (PADLright >> 8))) {
            gPadEvent = PAD_EVENT_DOWN_RIGHT;
        } else {
            gPadEvent = PAD_EVENT_DOWN;
        }
    } else {
        if (!(buttons & (PADLleft >> 8))) {
            gPadEvent = PAD_EVENT_LEFT;
        } else if (!(buttons & (PADLright >> 8))) {
            gPadEvent = PAD_EVENT_RIGHT;
        }
    }
}

void ReadPadEvent(void)
{
    int padType;
    short previous;

    gPadEvent = PAD_EVENT_NONE;
    if ((unsigned int)PadGetState(0) >= PadStateFindCTP1) {
        padType = (unsigned int)sPadBuffer[0][1] >> 4;
        if (sPadBuffer[0][0] == 0 && (padType == 4 || padType == 7)) {
            if (!(sPadBuffer[0][3] & PADRright)) {
                gPadEvent = PAD_EVENT_CIRCLE;
            } else if (!(sPadBuffer[0][3] & PADRdown)) {
                gPadEvent = PAD_EVENT_CROSS;
            } else if (!(sPadBuffer[0][2] & (PADstart >> 8))) {
                gPadEvent = PAD_EVENT_START;
            } else if (!(sPadBuffer[0][3] & PADRup) || !(sPadBuffer[0][2] & (PADselect >> 8))) {
                gPadEvent = PAD_EVENT_TRIANGLE;
            } else if (!(sPadBuffer[0][3] & PADRleft)) {
                gPadEvent = PAD_EVENT_SQUARE;
            } else if (!(sPadBuffer[0][3] & PADL1)) {
                gPadEvent = PAD_EVENT_L1;
            } else if (!(sPadBuffer[0][3] & PADL2)) {
                gPadEvent = PAD_EVENT_L2;
            } else if (!(sPadBuffer[0][3] & PADR1)) {
                gPadEvent = PAD_EVENT_R1;
            } else if (!(sPadBuffer[0][3] & PADR2)) {
                gPadEvent = PAD_EVENT_R2;
            } else if (padType == 7) {
                if (sPadBuffer[0][4] < 0x50) {
                    gPadEvent = PAD_EVENT_L1;
                } else if (sPadBuffer[0][4] >= 0xb1) {
                    gPadEvent = PAD_EVENT_R1;
                } else if (sPadBuffer[0][5] < 0x50) {
                    gPadEvent = PAD_EVENT_RIGHT_STICK_UP;
                } else if (sPadBuffer[0][5] >= 0xb1) {
                    gPadEvent = PAD_EVENT_RIGHT_STICK_DOWN;
                } else if (sPadBuffer[0][7] < 0x50) {
                    if (sPadBuffer[0][6] < 0x50) {
                        gPadEvent = PAD_EVENT_UP_LEFT;
                    } else if (sPadBuffer[0][6] >= 0xb1) {
                        gPadEvent = PAD_EVENT_UP_RIGHT;
                    } else {
                        gPadEvent = PAD_EVENT_UP;
                    }
                } else if (sPadBuffer[0][7] >= 0xb1) {
                    if (sPadBuffer[0][6] < 0x50) {
                        gPadEvent = PAD_EVENT_DOWN_LEFT;
                    } else if (sPadBuffer[0][6] >= 0xb1) {
                        gPadEvent = PAD_EVENT_DOWN_RIGHT;
                    } else {
                        gPadEvent = PAD_EVENT_DOWN;
                    }
                } else if (sPadBuffer[0][6] < 0x50) {
                    gPadEvent = PAD_EVENT_LEFT;
                } else if (sPadBuffer[0][6] >= 0xb1) {
                    gPadEvent = PAD_EVENT_RIGHT;
                } else {
                    ReadPadDirection();
                }
            } else {
                ReadPadDirection();
            }
        }
    }
    previous = gLastPadEvent;
    if (previous != PAD_EVENT_NONE && previous == gPadEvent) {
        gPadEvent = PAD_EVENT_NONE;
    } else {
        gLastPadEvent = gPadEvent;
    }
}

void InitPads(void)
{
    gPadEvent = PAD_EVENT_NONE;
    gLastPadEvent = PAD_EVENT_NONE;
    gSceneCommand = SCENE_CMD_NONE;
    PadInitDirect(sPadBuffer[0], sPadBuffer[1]);
    PadStartCom();
}

/* Grid cursor storage defined in pad's .sbss; read and written by grid_menu and grid_scene. */
short gCursorCell;
short gCursorLevel;
