#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern long RotAverageNclipColorDpq3_1(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2,
                                      SVECTOR *n0, SVECTOR *n1, SVECTOR *n2,
                                      CVECTOR *color, long *xy0, long *xy1, long *xy2,
                                      CVECTOR *c0, CVECTOR *c1, CVECTOR *c2,
                                      long *otz, long *flag);

extern POLY_GT4 *RCpolyGT4A(POLY_GT4 *packet, DIVPOLYGON4 *divp,
                         int depth, CRVECTOR4 *work);

PACKET *GsTMDdivTG4LFG(TMD_P_TG4 *prim, SVECTOR *vertices, SVECTOR *normals,
                      POLY_GT4 *packet, u_long count, u_long shift, GsOT *ot,
                      DIVPOLYGON4 *divp)
{
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    RVECTOR *r3 = &divp->r3;
    CRVECTOR4 *work = divp->cr;
    u_long i;
    union { u_long word; CVECTOR vector; } color;
    long otz, flag, p;

    /* 0x80 per channel is unit texture modulation; the GTE's lighting copies the code
       byte, 0x3c (POLY_GT4), into each lit colour. */
    color.word = 0x3c808080;
    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    divp->cr[0].r3 = r3;
    for (i = 0; i < count; prim++, i++) {
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        r3->v = vertices[prim->v3];
        if (RotAverageNclipColorDpq3_1(&r0->v, &r1->v, &r2->v,
                                     &normals[prim->n0], &normals[prim->n1], &normals[prim->n2],
                                     &color.vector,
                                     (long *)&r0->sxy, (long *)&r1->sxy, (long *)&r2->sxy,
                                     &r0->c, &r1->c, &r2->c, &otz, &flag) > 0) {
            RotTransPers(&r3->v, (long *)&r3->sxy, &p, &flag);
            ReadSZfifo4((long *)&r0->sz, (long *)&r1->sz,
                        (long *)&r2->sz, (long *)&r3->sz);
            NormalColorDpq(&normals[prim->n3], &color.vector, p, &r3->c);
            divp->ot = (u_long *)ot->org + (((u_short)otz - ot->offset) >> shift);
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            divp->clut = prim->clut;
            divp->tpage = prim->tpage;
            *(u_long *)r0->uv = *(u_long *)&prim->tu0;
            *(u_long *)r1->uv = *(u_long *)&prim->tu1;
            *(u_long *)r2->uv = *(u_long *)&prim->tu2;
            *(u_long *)r3->uv = *(u_long *)&prim->tu3;
            packet = RCpolyGT4A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
