#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

void GsGetLws(GsCOORDINATE2 *coord, MATRIX *world, MATRIX *screen)
{
    GsCOORDINATE2 *entry = coord;
    int depth = 0;
    int changed = 100;
    GsCOORDINATE2 *parent;

    for (;; entry = parent, depth++) {
        DSTACK[depth] = entry;
        parent = entry->super;
        if (parent == WORLD) {
            if (entry->flg == PSDCNT || entry->flg == 0) {
                entry->workm = entry->coord;
                *world = entry->workm;
                entry->flg = PSDCNT;
            } else if (changed == 100) {
                *world = DSTACK[0]->workm;
                depth = 0;
            } else {
                depth = changed + 1;
                *world = DSTACK[depth]->workm;
            }
            break;
        }
        if (entry->flg == PSDCNT) {
            *world = entry->workm;
            break;
        }
        if (entry->flg == 0)
            changed = depth;
    }

    while (depth > 0) {
        GsMulCoord3(world, &DSTACK[depth - 1]->coord);
        DSTACK[depth - 1]->workm = *world;
        DSTACK[depth - 1]->flg = PSDCNT;
        depth--;
    }
    *screen = *world;
    GsMulCoord2(&GsWSMATRIX, screen);
}
