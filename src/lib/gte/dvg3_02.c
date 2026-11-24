#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern POLY_G3 *RCpolyG3A(POLY_G3 *packet, DIVPOLYGON3 *divp,
                         int depth, CRVECTOR3 *work);

PACKET *GsTMDdivG3NL(TMD_P_G3 *prim, SVECTOR *vertices, SVECTOR *normals,
                     POLY_G3 *packet, u_long count, u_long shift, GsOT *ot,
                     DIVPOLYGON3 *divp)
{
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    CRVECTOR3 *work = divp->cr;
    u_long i;
    long p, otz, flag;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    for (i = 0; i < count; prim++, i++) {
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        if (RotAverageNclip3(&r0->v, &r1->v, &r2->v,
                            (long *)&r0->sxy, (long *)&r1->sxy, (long *)&r2->sxy,
                            &p, &otz, &flag) > 0) {
            ReadSZfifo3((long *)&r0->sz, (long *)&r1->sz, (long *)&r2->sz);
            divp->ot = (u_long *)ot->org + (((u_short)otz - ot->offset) >> shift);
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            *(u_long *)&r0->c = *(u_long *)&prim->r0;
            *(u_long *)&r1->c = *(u_long *)&prim->r0;
            *(u_long *)&r2->c = *(u_long *)&prim->r0;
            packet = RCpolyG3A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
