#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern long RotAverageNclipColorDpq3_1(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2,
                                      SVECTOR *n0, SVECTOR *n1, SVECTOR *n2,
                                      CVECTOR *color, long *xy0, long *xy1, long *xy2,
                                      CVECTOR *c0, CVECTOR *c1, CVECTOR *c2,
                                      long *otz, long *flag);

extern POLY_G3 *RCpolyG3A(POLY_G3 *packet, DIVPOLYGON3 *divp,
                         int depth, CRVECTOR3 *work);

PACKET *GsTMDdivG3LFG(TMD_P_G3 *prim, SVECTOR *vertices, SVECTOR *normals,
                     POLY_G3 *packet, u_long count, u_long shift, GsOT *ot,
                     DIVPOLYGON3 *divp)
{
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    CRVECTOR3 *work = divp->cr;
    u_long i;
    long otz, flag;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    for (i = 0; i < count; prim++, i++) {
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        if (RotAverageNclipColorDpq3_1(&r0->v, &r1->v, &r2->v,
                                     &normals[prim->n0], &normals[prim->n1], &normals[prim->n2],
                                     (CVECTOR *)&prim->r0,
                                     (long *)&r0->sxy, (long *)&r1->sxy, (long *)&r2->sxy,
                                     &r0->c, &r1->c, &r2->c, &otz, &flag) > 0) {
            ReadSZfifo3((long *)&r0->sz, (long *)&r1->sz, (long *)&r2->sz);
            divp->ot = (u_long *)ot->org + (((u_short)otz - ot->offset) >> shift);
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            packet = RCpolyG3A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
