#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern POLY_FT3 *RCpolyFT3A(POLY_FT3 *packet, DIVPOLYGON3 *divp,
                           int depth, CRVECTOR3 *work);

PACKET *GsTMDdivTF3L(TMD_P_TF3 *prim, SVECTOR *vertices, SVECTOR *normals,
                    POLY_FT3 *packet, u_long count, u_long shift, GsOT *ot,
                    DIVPOLYGON3 *divpTop)
{
    DIVPOLYGON3 *divp = divpTop;
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    CRVECTOR3 *work = divp->cr;
    struct {
        CVECTOR color;
        union {
            long word;
            u_short half;
        } otz;
        long flag;
        long p;
    } gte;
    u_long i;

    /* 0x80 per channel is unit texture modulation; the GTE's lighting copies the code
       byte, 0x24 (POLY_FT3), into each lit colour. */
    *(u_long *)&gte.color = 0x24808080;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;

    i = 0;
    if (count == 0)
        return (PACKET *)packet;

    do {
        long *sxy1 = (long *)&divp->r1.sxy;
        long *sxy2 = (long *)&divp->r2.sxy;
        CVECTOR *rgb0 = &divp->rgbc;
        CVECTOR *rgb1 = &divp->r1.c;
        CVECTOR *rgb2 = &divp->r2.c;
        long *flagOut = &gte.flag;

        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];

        /* TMD primitive flag bit 1 disables back-face culling. */
        if (!(prim->dummy & 2)) {
            if (RotAverageNclipColorCol3(&r0->v, &r1->v, &r2->v,
                     &normals[prim->n0], &normals[prim->n0], &normals[prim->n0], &gte.color,
                     (long *)&r0->sxy, sxy1, sxy2,
                     rgb0, rgb1, rgb2, &gte.otz.word, flagOut) <= 0)
                continue;
        } else {
            u_short depth = RotAverage3(&r0->v, &r1->v, &r2->v,
                                      (long *)&r0->sxy, sxy1, sxy2,
                                      &gte.p, flagOut);
            SVECTOR *normal = &normals[prim->n0];
            gte.otz.half = depth;
            NormalColorCol3(normal, normal, normal, &gte.color, rgb0, rgb1, rgb2);
        }

        ReadSZfifo3((long *)&r0->sz, (long *)&r1->sz, (long *)&r2->sz);
        divp->ot = (u_long *)ot->org + ((gte.otz.half - ot->offset) >> shift);
        divp->rgbc.cd = prim->cd | (GsTON << 1);
        divp->clut = prim->clut;
        divp->tpage = prim->tpage;
        *(u_long *)r0->uv = *(u_long *)&prim->tu0;
        *(u_long *)r1->uv = *(u_long *)&prim->tu1;
        *(u_long *)r2->uv = *(u_long *)&prim->tu2;
        packet = RCpolyFT3A(packet, divp, 0, work);
    } while (prim++, ++i < count);
    return (PACKET *)packet;
}
