/* Post-end-roll continue prompt: loads BIN.BIN part 31 ("end" and "continue" over fading eyes and
 * static) and returns GRID_MENU_EXIT_START or GRID_MENU_EXIT_CONTINUE. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <rand.h>
#include "about.h"
#include "graphics.h"
#include "grid_menu.h"
#include "pad.h"
#include "sound.h"
#include "system_menu.h"
#include "continue_prompt.h"

static SpriteQuad sSpinningQuad;
GsSPRITE gContinuePromptSprites[15] = {
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xa0, 0xc0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 0 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xb0, 0xc0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 1 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xc0, 0xc0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 2 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xd0, 0xc0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 3 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xa0, 0xd0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 4 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xb0, 0xd0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 5 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xc0, 0xd0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 6 */
    { 0x00, 152, 88, 0x10, 0x10, getTPage(0, 0, 768, 256), 0xd0, 0xd0, 48, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* cursor frame 7 */
    { 0x00, 136, 72, 0x30, 0x10, getTPage(0, 0, 768, 256), 0xb0, 0xa8, 32, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* "end" */
    { 0x00, 128, 144, 0x40, 0x08, getTPage(0, 0, 768, 256), 0xa0, 0xb8, 32, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* "continue" */
    { 0x40000000, 74, 113, 0xb8, 0x10, getTPage(0, 1, 768, 256), 0x00, 0x98, 0, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* eyes, faded in and out */
    { 0x00, 0, 113, 0xa0, 0x0e, getTPage(0, 0, 768, 256), 0x00, 0xa8, 16, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* static */
    { 0x00, 0, 113, 0xa0, 0x0e, getTPage(0, 0, 768, 256), 0x00, 0xb6, 16, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* static */
    { 0x00, 0, 113, 0xa0, 0x0e, getTPage(0, 0, 768, 256), 0x00, 0xc4, 16, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* static */
    { 0x00, 0, 113, 0xa0, 0x0e, getTPage(0, 0, 768, 256), 0x00, 0xd2, 16, 488, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 }, /* static */
};

enum ContinueChoice {
    CONTINUE_CHOICE_END,
    CONTINUE_CHOICE_CONTINUE
};

int RunContinuePrompt(void)
{
    RECT rect;
    int angle;
    int choice;
    int cursorFrame;
    int staticFrame;
    int eyesTimer;
    int i;
    int nextStaticFrame;
    int result;

    LoadTimPartAt(31, 768, 408, 0, 488);
    setRECT(&rect, 0, 0, 320, 480);
    ClearImage(&rect, 0, 0, 0);
    angle = 0;
    choice = CONTINUE_CHOICE_END;
    cursorFrame = 0;
    staticFrame = 0;
    eyesTimer = 0;

    sSpinningQuad.tpage = getTPage(0, 0, 768, 256);
    sSpinningQuad.clut = getClut(32, 488);
    sSpinningQuad.scale[0] = 4096;
    sSpinningQuad.scale[1] = 4096;
    sSpinningQuad.x = 152;
    sSpinningQuad.y = 112;
    sSpinningQuad.w = 16;
    sSpinningQuad.h = 16;
    sSpinningQuad.u = 160;
    sSpinningQuad.v = 168;
    sSpinningQuad.attribute = 0;
    sSpinningQuad.rotateX = 0;
    sSpinningQuad.rotateY = 0;
    sSpinningQuad.rotateZ = 0;
    sSpinningQuad.r = 128;
    sSpinningQuad.g = 128;
    sSpinningQuad.b = 128;
    for (;;) {
        DrawSync(0);
        BeginFrame();
        ReadPadEvent();
        switch (gPadEvent) {
        case PAD_EVENT_UP: case PAD_EVENT_UP_LEFT: case PAD_EVENT_UP_RIGHT:
            PlaySound(1);
            choice = CONTINUE_CHOICE_END;
            break;
        case PAD_EVENT_DOWN: case PAD_EVENT_DOWN_LEFT: case PAD_EVENT_DOWN_RIGHT:
            PlaySound(1);
            choice = CONTINUE_CHOICE_CONTINUE;
            break;
        case PAD_EVENT_CIRCLE:
            PlaySound(0);
            if (choice == CONTINUE_CHOICE_END) {
                result = GRID_MENU_EXIT_START;
            } else {
                result = GRID_MENU_EXIT_CONTINUE;
            }
            goto exit;
        }
        if (choice == CONTINUE_CHOICE_END) {
            gContinuePromptSprites[cursorFrame].y = 88;
        } else {
            gContinuePromptSprites[cursorFrame].y = 152;
        }
        GsSortFastSprite(&gContinuePromptSprites[cursorFrame], &gSubOt[gActiveBuffer], 4);
        if (eyesTimer != 0) {
            if (eyesTimer <= 16) {
                u_char shade = 128 - (16 - eyesTimer) * 8;
                gContinuePromptSprites[10].r = shade;
                gContinuePromptSprites[10].g = shade;
                gContinuePromptSprites[10].b = shade;
            } else if (eyesTimer >= 164) {
                u_char shade = (180 - eyesTimer) * 8;
                gContinuePromptSprites[10].r = shade;
                gContinuePromptSprites[10].g = shade;
                gContinuePromptSprites[10].b = shade;
            }
            GsSortFastSprite(&gContinuePromptSprites[10], &gSubOt[gActiveBuffer], 5);
            eyesTimer--;
        } else if (rand() % 500 == 0) {
            eyesTimer = 180;
        }
        for (i = 9; i >= 8; i--) {
            GsSortFastSprite(&gContinuePromptSprites[i], &gSubOt[gActiveBuffer], 5);
        }
        nextStaticFrame = staticFrame + 1;
        if (nextStaticFrame >= 4) {
            nextStaticFrame = 0;
        }
        gContinuePromptSprites[staticFrame + 11].x = 0;
        gContinuePromptSprites[nextStaticFrame + 11].x = 160;
        GsSortFastSprite(&gContinuePromptSprites[staticFrame + 11], &gSubOt[gActiveBuffer], 6);
        GsSortFastSprite(&gContinuePromptSprites[nextStaticFrame + 11], &gSubOt[gActiveBuffer], 6);
        sSpinningQuad.rotateZ = angle;
        SortRotatedSpriteQuad(&sSpinningQuad, &gSubOt[gActiveBuffer], 4);
        VSync(0);
        VSync(0);
        ResetGraph(1);
        angle += 16;
        GsSwapDispBuff();
        GsSortClear(0, 0, 0, &gSubOt[gActiveBuffer]);
        GsDrawOt(&gSubOt[gActiveBuffer]);
        if (angle >= 4096) {
            angle = 0;
        }
        if ((angle & 31) == 0) {
            if (++cursorFrame >= 8) {
                cursorFrame = 0;
            }
        }
        if (++staticFrame >= 4) {
            staticFrame = 0;
        }
    }
exit:
    setRECT(&rect, 0, 0, 320, 480);
    ClearImage(&rect, 0, 0, 0);
    return result;
}
