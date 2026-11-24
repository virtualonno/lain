#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern POLY_FT3 *RCpolyFT3A(POLY_FT3 *packet, DIVPOLYGON3 *divp,
                           int depth, CRVECTOR3 *work);

PACKET *GsTMDdivTF3NL(TMD_P_TF3 *prim, SVECTOR *vertices, SVECTOR *normals,
                     POLY_FT3 *packet, u_long count, u_long shift, GsOT *ot,
                     DIVPOLYGON3 *divpTop)
{
    u_long color = 0x24808080;
    DIVPOLYGON3 *divp = divpTop;
    RVECTOR *r0 = &divp->r0;
    RVECTOR *r1 = &divp->r1;
    RVECTOR *r2 = &divp->r2;
    CRVECTOR3 *work = divp->cr;
    u_long i;
    struct {
        long p;
        union {
            long word;
            u_short half;
        } otz;
    } proj;
    long flag;

    divp->cr[0].r0 = r0;
    divp->cr[0].r1 = r1;
    divp->cr[0].r2 = r2;
    for (i = 0; i < count; prim++, i++) {
        long *sxy1 = (long *)&r1->sxy;
        long *sxy2 = (long *)&r2->sxy;
        long *flagp, *pp;

        pp = &proj.p;
        flagp = &flag;
        r0->v = vertices[prim->v0];
        r1->v = vertices[prim->v1];
        r2->v = vertices[prim->v2];
        /* TMD primitive flag bit 1 disables back-face culling. */
        if (!(prim->dummy & 2)) {
            if (RotAverageNclip3(&r0->v, &r1->v, &r2->v,
                                 (long *)&r0->sxy, sxy1, sxy2,
                                 pp, &proj.otz.word, flagp) > 0) {
                ReadSZfifo3((long *)&r0->sz, (long *)&r1->sz, (long *)&r2->sz);
                divp->ot = (u_long *)ot->org + ((proj.otz.half - ot->offset) >> shift);
                *(u_long *)&divp->rgbc = color;
                divp->rgbc.cd = prim->cd | (GsTON << 1);
                divp->clut = prim->clut;
                divp->tpage = prim->tpage;
                *(u_long *)r0->uv = *(u_long *)&prim->tu0;
                *(u_long *)r1->uv = *(u_long *)&prim->tu1;
                *(u_long *)r2->uv = *(u_long *)&prim->tu2;
                packet = RCpolyFT3A(packet, divp, 0, work);
            }
        } else {
            proj.otz.half = RotAverage3(&r0->v, &r1->v, &r2->v,
                                        (long *)&r0->sxy, sxy1, sxy2,
                                        pp, flagp);
            ReadSZfifo3((long *)&r0->sz, (long *)&r1->sz, (long *)&r2->sz);
            divp->ot = (u_long *)ot->org + ((proj.otz.half - ot->offset) >> shift);
            *(u_long *)&divp->rgbc = color;
            divp->rgbc.cd = prim->cd | (GsTON << 1);
            divp->clut = prim->clut;
            divp->tpage = prim->tpage;
            *(u_long *)r0->uv = *(u_long *)&prim->tu0;
            *(u_long *)r1->uv = *(u_long *)&prim->tu1;
            *(u_long *)r2->uv = *(u_long *)&prim->tu2;
            packet = RCpolyFT3A(packet, divp, 0, work);
        }
    }
    return (PACKET *)packet;
}
