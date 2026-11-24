/* GsSortSprite adds a sprite to an ordering table: a texture-page word and a SPRT when GsROTOFF
   is set or it is unscaled, unrotated and unflipped, else a POLY_FT4 the GTE places. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

void GsSortSprite(GsSPRITE *sp, GsOT *ot, unsigned short pri)
{
    unsigned long attr;
    unsigned long *packet;
    int untransformed, rotationOff;
    MATRIX matrix;
    SVECTOR rotation;
    VECTOR vector;
    SVECTOR tl, tr, bl, br;
    long sxy0, sxy1, sxy2, sxy3, p, flag;
    int x, y;
    int left, top;
    unsigned char u0, u1, v0, v1;

    attr = sp->attribute;
    if ((attr & GsDOFF) || !sp->w || !sp->h)
        return;

    untransformed = 0;
    packet = (unsigned long *)GsOUT_PACKET_P;
    rotationOff = (attr >> 27) & 1;
    /* scalex and scaley both 4096, 1.0 */
    if (*(long *)&sp->scalex == 0x10001000 && !sp->rotate) {
        unsigned long flip = attr & 0xc00000;
        untransformed = flip == 0;
    }

    if (rotationOff | untransformed) {
        x = sp->x + GsORGOFSX;
        y = sp->y + GsORGOFSY;
        packet[1] = ((sp->tpage & 0x1f) | 0xe1000200) | ((attr >> 17) & 0x180) | ((attr >> 23) & 0x60);
        packet[2] = (((attr >> 5) & 0x02000000) | 0x64000000) | ((attr << 18) & 0x01000000) | (sp->b << 16) | (sp->g << 8) | sp->r;
        packet[3] = ((x - sp->mx) & 0xffff) | ((unsigned long)(y - sp->my) << 16);
        packet[4] = sp->u | (sp->v << 8) | ((unsigned long)sp->cy << 22) | (((unsigned long)sp->cx << 12) & 0x3f0000);
        packet[5] = sp->w | ((unsigned long)sp->h << 16);
        packet = (unsigned long *)_make_packet((PACKET *)packet, ot, pri, 5);
    } else {
        if (!sp->rotate) {
            matrix = GsIDMATRIX;
        } else {
            rotation.vx = 0;
            rotation.vy = 0;
            rotation.vz = sp->rotate / 360;
            RotMatrix(&rotation, &matrix);
        }
        if (*(long *)&sp->scalex != 0x10001000) {
            vector.vx = sp->scalex;
            vector.vy = sp->scaley;
            vector.vz = 0;
            ScaleMatrix(&matrix, &vector);
        }
        vector.vx = sp->x;
        vector.vy = sp->y;
        vector.vz = ReadGeomScreen();
        TransMatrix(&matrix, &vector);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);

        left = -sp->mx;
        top = -sp->my;
        tl.vz = 0;
        tl.vx = left;
        tl.vy = top;
        tr.vx = sp->w + left;
        tr.vy = top;
        tr.vz = 0;
        bl.vx = left;
        bl.vy = sp->h + top;
        bl.vz = 0;
        br.vx = sp->w + left;
        br.vy = sp->h + top;
        br.vz = 0;
        RotTransPers4(&tl, &tr, &bl, &br, &sxy0, &sxy1, &sxy2, &sxy3, &p, &flag);

        if (attr & 0x800000) {
            u0 = sp->u + sp->w - 1;
            u1 = sp->u;
        } else {
            u0 = sp->u;
            u1 = u0 + sp->w - 1;
        }
        if (attr & 0x400000) {
            v0 = sp->v + sp->h - 1;
            v1 = sp->v;
        } else {
            v0 = sp->v;
            v1 = v0 + sp->h - 1;
        }

        packet[1] = (((attr >> 5) & 0x02000000) | 0x2c000000) | ((attr << 18) & 0x01000000) | (sp->b << 16) | (sp->g << 8) | sp->r;
        packet[2] = sxy0;
        packet[3] = u0 | (v0 << 8) | ((unsigned long)sp->cy << 22) | (((unsigned long)sp->cx << 12) & 0x3f0000);
        packet[4] = sxy1;
        packet[5] = u1 | (v0 << 8) | ((sp->tpage & 0x1f) << 16) | ((attr >> 1) & 0x01800000) | ((attr >> 7) & 0x00600000);
        packet[6] = sxy2;
        packet[7] = u0 | (v1 << 8);
        packet[8] = sxy3;
        packet[9] = u1 | (v1 << 8);
        packet = (unsigned long *)_make_packet((PACKET *)packet, ot, pri, 9);
    }
    GsOUT_PACKET_P = (PACKET *)packet;
}
