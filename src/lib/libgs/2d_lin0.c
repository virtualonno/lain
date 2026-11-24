#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

typedef struct {
    u_long tag, mode;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
} DrawLine;

void GsSortLine(GsLINE *line, GsOT *ot, u_short pri)
{
    long attr = line->attribute;
    if (attr >= 0) {
        DrawLine *p = (DrawLine *)GsOUT_PACKET_P;
        u_char blue;
        p->mode = _get_mode(0, 1, getTPage(0, attr >> 28, 0, 0));
        p->r0 = line->r;
        p->g0 = line->g;
        blue = line->b;
        p->code = 0x40 | ((attr >> 29) & 2);
        p->b0 = blue;
        p->x0 = line->x0 + GsORGOFSX;
        p->y0 = line->y0 + GsORGOFSY;
        p->x1 = line->x1 + GsORGOFSX;
        p->y1 = line->y1 + GsORGOFSY;
        GsOUT_PACKET_P = _make_packet((PACKET *)p, ot, pri, 4);
    }
}
