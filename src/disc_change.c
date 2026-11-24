/* Disc-change prompt and sector-table swap: draws the BIN.BIN part 24 prompt
 * until PollDiscChange and IdentifyDisc see the expected site disc, then
 * copies gDisc1Files or gDisc2Files into gDiscFiles. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include "disc_change.h"
#include "disc_id.h"
#include "sprite_quad.h"
#include "graphics.h"
#include "grid_menu.h"
#include "media_player.h"
#include "system_menu.h"

static SpriteQuad sDiscChangeLines[24];

GsSPRITE gDiscChangeSprites[10] = {
    { 0x00, 48, 64, 0xe0, 0x18, getTPage(0, 2, 768, 256), 0x00, 0x00, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "DISC Change Request" */
    { 0x00, 96, 64, 0x88, 0x18, getTPage(0, 2, 768, 256), 0x00, 0x18, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "Change SITE" */
    { 0x00, 96, 125, 0x88, 0x10, getTPage(0, 2, 768, 256), 0x00, 0x30, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "Please insert" */
    { 0x00, 96, 146, 0x68, 0x20, getTPage(0, 2, 768, 256), 0x00, 0x70, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "DISC" */
    { 0x00, 208, 146, 0x18, 0x20, getTPage(0, 2, 768, 256), 0xa0, 0x30, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "1" */
    { 0x00, 208, 146, 0x18, 0x20, getTPage(0, 2, 768, 256), 0xb8, 0x30, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "2" */
    { 0x00, 80, 99, 0xa0, 0x10, getTPage(0, 2, 768, 256), 0x00, 0x40, 16, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "WRONG DISC" */
    { 0x00, 50, 125, 0xd8, 0x18, getTPage(0, 2, 768, 256), 0x00, 0x58, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "checking in progress" */
    { 0x00, 108, 99, 0x68, 0x10, getTPage(0, 2, 768, 256), 0x88, 0x18, 16, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* "No DISC" */
    { 0x00, 126, 20, 0x60, 0x20, getTPage(0, 2, 768, 256), 0x68, 0x70, 0, 487, 0x80, 0x80, 0x80, 0, 0, 4096, 4096, 0 }, /* lain logo */
};
short gDiscChangeLinePos[24][2] = {
    { 16, 84 },
    { 160, 84 },
    { 16, 88 },
    { 160, 88 },
    { 16, 146 },
    { 160, 146 },
    { 16, 150 },
    { 160, 150 },
    { 16, 154 },
    { 160, 154 },
    { 16, 158 },
    { 160, 158 },
    { 16, 162 },
    { 160, 162 },
    { 16, 166 },
    { 160, 166 },
    { 16, 170 },
    { 160, 170 },
    { 16, 174 },
    { 160, 174 },
    { 16, 174 },
    { 160, 174 },
    { 16, 178 },
    { 160, 178 },
};

void DrawDiscChangeScreen(int siteChange, int checked, int status)
{
    int i;
    int show;

    for (i = 0; i < 24; i++) {
        if (i & 1) {
            sDiscChangeLines[i].attribute = 0x40800000;
        } else {
            sDiscChangeLines[i].attribute = 0x40000000;
        }
        sDiscChangeLines[i].tpage = getTPage(0, 2, 768, 256);
        sDiscChangeLines[i].clut = getClut(0, 487);
        sDiscChangeLines[i].x = gDiscChangeLinePos[i][0];
        sDiscChangeLines[i].y = gDiscChangeLinePos[i][1];
        if (i < 20) {
            sDiscChangeLines[i].u = 0;
            sDiscChangeLines[i].v = 0x50;
        } else {
            sDiscChangeLines[i].u = 0;
            sDiscChangeLines[i].v = 0x90;
        }
        sDiscChangeLines[i].w = 0x90;
        sDiscChangeLines[i].h = 8;
        sDiscChangeLines[i].r = 0x80;
        sDiscChangeLines[i].g = 0x80;
        sDiscChangeLines[i].b = 0x80;
    }
    DrawSync(0);
    BeginFrame();
    for (i = 0; i < 10; i++) {
        show = 1;
        switch (i) {
        case 0:
            if (siteChange != 0) {
                show = 0;
            }
            break;
        case 1:
            if (siteChange == 0) {
                show = 0;
            }
            break;
        case 2:
            if (status == DISC_POLL_CHECKING) {
                show = 0;
            }
            break;
        case 3:
            break;
        case 4:
            if (gCurrentDisc != 0) {
                show = 0;
            }
            break;
        case 5:
            if (gCurrentDisc == 0) {
                show = 0;
            }
            break;
        case 6:
            show = checked != 0 && status == DISC_POLL_WRONG_DISC;
            break;
        case 7:
            if (status != DISC_POLL_CHECKING) {
                show = 0;
            }
            break;
        case 8:
            show = checked != 0 && status == DISC_POLL_NO_DISC;
            break;
        }
        if (show) {
            GsSortFastSprite(&gDiscChangeSprites[i], &gSubOt[gActiveBuffer], 1);
        }
    }
    for (i = 0; i < 24; i++) {
        SortSpriteQuad(&sDiscChangeLines[i], &gSubOt[gActiveBuffer], 1);
    }
    VSync(0);
    ResetGraph(1);
    GsSwapDispBuff();
    GsSortClear(0xff, 0xff, 0xff, &gSubOt[gActiveBuffer]);
    GsDrawOt(&gSubOt[gActiveBuffer]);
}

void EnsureSiteDisc(int siteChange)
{
    int checked;
    int status;
    int n;

    if (IdentifyDisc() != gCurrentDisc) {
        checked = 0;
        status = DISC_POLL_WAIT_LID;
        for (;;) {
            do {
                DrawDiscChangeScreen(siteChange, checked, status);
                n = PollDiscChange();
            } while (n == DISC_POLL_WAIT_LID);
            if (n == DISC_POLL_CHECKING) {
                do {
                    DrawDiscChangeScreen(siteChange, checked, n);
                    n = PollDiscChange();
                } while (n == DISC_POLL_CHECKING);
            }
            if (n == DISC_POLL_READY) {
                if (IdentifyDisc() == gCurrentDisc) {
                    break;
                }
                n = DISC_POLL_WRONG_DISC;
            }
            checked = 1;
            status = n;
        }
    }
    /* copies 100 entries; disc 1's table holds 69 and disc 2's 57, so the rest is the storage after each */
    if (gCurrentDisc == 0) {
        for (n = 0; n < 100; n++) {
            gDiscFiles[n].sector = gDisc1Files[n].sector;
            gDiscFiles[n].size = gDisc1Files[n].size;
        }
    } else {
        for (n = 0; n < 100; n++) {
            gDiscFiles[n].sector = gDisc2Files[n].sector;
            gDiscFiles[n].size = gDisc2Files[n].size;
        }
    }
}
