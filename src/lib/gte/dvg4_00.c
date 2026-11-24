#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern POLY_G4 *RCpolyG4A(POLY_G4 *packet, DIVPOLYGON4 *divp,
                         int depth, CRVECTOR4 *work);

PACKET *GsTMDdivG4L(TMD_P_G4 *prim, SVECTOR *vertices, SVECTOR *normals,
                    POLY_G4 *packet,
                    u_long count, u_long shift, GsOT *ot,
                    DIVPOLYGON4 *divp)
{
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    RVECTOR *r3 = &divp->r3;
    u_long i;
    CRVECTOR4 *work;
    long p, otz, flag;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    divp->cr[0].r3 = r3;
    work = divp->cr;
    for (i = 0; i < count; prim++, i++) {
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        r3->v = vertices[prim->v3];
        if (RotAverageNclip4(&r0->v, &r1->v, &r2->v, &r3->v,
                            (long *)&r0->sxy, (long *)&r1->sxy,
                            (long *)&r2->sxy, (long *)&r3->sxy,
                            &p, &otz, &flag) > 0) {
            CVECTOR *rgb;

            ReadSZfifo4((long *)&r0->sz, (long *)&r1->sz,
                        (long *)&r2->sz, (long *)&r3->sz);
            rgb = (CVECTOR *)&prim->r0;
            NormalColorCol3(&normals[prim->n0], &normals[prim->n1], &normals[prim->n2],
                            rgb, &r0->c, &r1->c, &r2->c);
            NormalColorCol(&normals[prim->n3], rgb, &r3->c);
            divp->ot = (u_long *)ot->org + (((u_short)otz - ot->offset) >> shift);
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            packet = RCpolyG4A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
