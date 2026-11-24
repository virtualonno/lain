#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

typedef struct {
    u_long tag, mode;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, pad;
    short x1, y1;
} DrawGLine;

void GsSortGLine(GsGLINE *line, GsOT *ot, u_short depth)
{
    long attr = line->attribute;
    if (!(attr & GsDOFF)) {
        DrawGLine *p = (DrawGLine *)GsOUT_PACKET_P;
        p->mode = 0xe1000200 | ((attr >> 23) & 0x60);
        p->r0 = line->r0;
        p->g0 = line->g0;
        p->b0 = line->b0;
        p->code = 0x50 | ((attr >> 29) & 2);
        p->x0 = line->x0 + GsORGOFSX;
        p->y0 = line->y0 + GsORGOFSY;
        p->r1 = line->r1;
        p->g1 = line->g1;
        p->b1 = line->b1;
        p->x1 = line->x1 + GsORGOFSX;
        p->y1 = line->y1 + GsORGOFSY;
        GsOUT_PACKET_P = _make_packet((PACKET *)p, ot, depth, 5);
    }
}
