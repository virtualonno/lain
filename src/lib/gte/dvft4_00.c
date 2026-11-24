#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern POLY_FT4 *RCpolyFT4A(POLY_FT4 *packet, DIVPOLYGON4 *divp,
                         int depth, CRVECTOR4 *work);

PACKET *GsTMDdivTF4L(TMD_P_TF4 *prim, SVECTOR *vertices, SVECTOR *normals,
                    POLY_FT4 *packet,
                    u_long count, u_long shift, GsOT *ot,
                    DIVPOLYGON4 *divp)
{
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    RVECTOR *r3 = &divp->r3;
    CRVECTOR4 *work = divp->cr;
    u_long i;
    long p, otz, flag;
    /* 0x80 per channel is unit texture modulation; the GTE's lighting copies the code
       byte, 0x2c (POLY_FT4), into each lit colour. */
    u_long color = 0x2c808080;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    divp->cr[0].r3 = r3;
    for (i = 0; i < count; prim++, i++) {
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        r3->v = vertices[prim->v3];
        if (RotAverageNclip4(&r0->v, &r1->v, &r2->v, &r3->v,
                            (long *)&r0->sxy, (long *)&r1->sxy,
                            (long *)&r2->sxy, (long *)&r3->sxy,
                            &p, &otz, &flag) > 0) {
            ReadSZfifo4((long *)&r0->sz, (long *)&r1->sz,
                        (long *)&r2->sz, (long *)&r3->sz);
            NormalColorCol(&normals[prim->n0], (CVECTOR *)&color, &divp->rgbc);
            divp->ot = (u_long *)ot->org + (((u_short)otz - ot->offset) >> shift);
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            divp->clut = prim->clut;
            divp->tpage = prim->tpage;
            *(u_long *)r0->uv = *(u_long *)&prim->tu0;
            *(u_long *)r1->uv = *(u_long *)&prim->tu1;
            *(u_long *)r2->uv = *(u_long *)&prim->tu2;
            *(u_long *)r3->uv = *(u_long *)&prim->tu3;
            packet = RCpolyFT4A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
