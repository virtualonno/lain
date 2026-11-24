/* GsSortObject4: sorts a GsDOBJ2's TMD object into an OT through the GsFCALL4 table.
   The primitives come in runs of one kind; a run's first word holds its length (low half),
   flag byte and mode byte, whose bit 1 (semi-transparency) the switch ignores. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>

PACKET *GsTMDfastF3L(), *GsTMDfastF3LFG(), *GsTMDfastF3NL(), *GsTMDfastNF3();
PACKET *GsTMDdivF3L(), *GsTMDdivF3LFG(), *GsTMDdivF3NL(), *GsTMDdivNF3();
PACKET *GsTMDfastG3L(), *GsTMDfastG3LFG(), *GsTMDfastG3NL(), *GsTMDfastNG3();
PACKET *GsTMDdivG3L(), *GsTMDdivG3LFG(), *GsTMDdivG3NL(), *GsTMDdivNG3();
PACKET *GsTMDfastTF3L(), *GsTMDfastTF3LFG(), *GsTMDfastTF3NL(), *GsTMDfastTNF3();
PACKET *GsTMDdivTF3L(), *GsTMDdivTF3LFG(), *GsTMDdivTF3NL(), *GsTMDdivTNF3();
PACKET *GsTMDfastTG3L(), *GsTMDfastTG3LFG(), *GsTMDfastTG3NL(), *GsTMDfastTNG3();
PACKET *GsTMDdivTG3L(), *GsTMDdivTG3LFG(), *GsTMDdivTG3NL(), *GsTMDdivTNG3();
PACKET *GsTMDfastF4L(), *GsTMDfastF4LFG(), *GsTMDfastF4NL(), *GsTMDfastNF4();
PACKET *GsTMDdivF4L(), *GsTMDdivF4LFG(), *GsTMDdivF4NL(), *GsTMDdivNF4();
PACKET *GsTMDfastG4L(), *GsTMDfastG4LFG(), *GsTMDfastG4NL(), *GsTMDfastNG4();
PACKET *GsTMDdivG4L(), *GsTMDdivG4LFG(), *GsTMDdivG4NL(), *GsTMDdivNG4();
PACKET *GsTMDfastTF4L(), *GsTMDfastTF4LFG(), *GsTMDfastTF4NL(), *GsTMDfastTNF4();
PACKET *GsTMDdivTF4L(), *GsTMDdivTF4LFG(), *GsTMDdivTF4NL(), *GsTMDdivTNF4();
PACKET *GsTMDfastTG4L(), *GsTMDfastTG4LFG(), *GsTMDfastTG4NL(), *GsTMDfastTNG4();
PACKET *GsTMDdivTG4L(), *GsTMDdivTG4LFG(), *GsTMDdivTG4NL(), *GsTMDdivTNG4();

static _GsFCALL GsFCALL4 = {
    {{GsTMDfastF3L, GsTMDfastF3LFG, GsTMDfastF3NL}, {GsTMDdivF3L, GsTMDdivF3LFG, GsTMDdivF3NL}},
    {GsTMDfastNF3, GsTMDdivNF3},
    {{GsTMDfastG3L, GsTMDfastG3LFG, GsTMDfastG3NL}, {GsTMDdivG3L, GsTMDdivG3LFG, GsTMDdivG3NL}},
    {GsTMDfastNG3, GsTMDdivNG3},
    {{GsTMDfastTF3L, GsTMDfastTF3LFG, GsTMDfastTF3NL}, {GsTMDdivTF3L, GsTMDdivTF3LFG, GsTMDdivTF3NL}},
    {GsTMDfastTNF3, GsTMDdivTNF3},
    {{GsTMDfastTG3L, GsTMDfastTG3LFG, GsTMDfastTG3NL}, {GsTMDdivTG3L, GsTMDdivTG3LFG, GsTMDdivTG3NL}},
    {GsTMDfastTNG3, GsTMDdivTNG3},
    {{GsTMDfastF4L, GsTMDfastF4LFG, GsTMDfastF4NL}, {GsTMDdivF4L, GsTMDdivF4LFG, GsTMDdivF4NL}},
    {GsTMDfastNF4, GsTMDdivNF4},
    {{GsTMDfastG4L, GsTMDfastG4LFG, GsTMDfastG4NL}, {GsTMDdivG4L, GsTMDdivG4LFG, GsTMDdivG4NL}},
    {GsTMDfastNG4, GsTMDdivNG4},
    {{GsTMDfastTF4L, GsTMDfastTF4LFG, GsTMDfastTF4NL}, {GsTMDdivTF4L, GsTMDdivTF4LFG, GsTMDdivTF4NL}},
    {GsTMDfastTNF4, GsTMDdivTNF4},
    {{GsTMDfastTG4L, GsTMDfastTG4LFG, GsTMDfastTG4NL}, {GsTMDdivTG4L, GsTMDdivTG4LFG, GsTMDdivTG4NL}},
    {GsTMDfastTNG4, GsTMDdivTNG4},
    {GsTMDfastF3GL, GsTMDfastF3GLFG, GsTMDfastF3GNL},
    {GsTMDfastG3GL, GsTMDfastG3GLFG, GsTMDfastG3GNL},
    {GsTMDfastF4GL, GsTMDfastF4GLFG, GsTMDfastF4GNL},
    {GsTMDfastG4GL, GsTMDfastG4GLFG, GsTMDfastG4GNL},
};
void GsSortObject4(GsDOBJ2 *objp, GsOT *ot, int shift, u_long *scratch)
{
    struct TMD_STRUCT *tmd;
    u_long *vertop, *nortop, *primtop;
    u_long primn;
    int ndiv, lmode;

    if (objp->attribute & GsDOFF)
        return;
    GsMATE_C = objp->attribute & 7;
    GsLMODE = (objp->attribute >> 3) & 3;
    GsLIGNR = (objp->attribute >> 5) & 1;
    GsLIOFF = (objp->attribute >> 6) & 1;
    GsNDIV = (objp->attribute >> 9) & 7;
    GsTON = (objp->attribute >> 30) & 1;
    if (GsNDIV) {
        /* ndiv, pih, piv: the head of the DIVPOLYGON3/4 the GsTMDdiv routines take */
        scratch[1] = HWD0;
        scratch[2] = VWD0;
        scratch[0] = GsNDIV;
        ndiv = GsDivMODE_DIV;
    } else
        ndiv = GsDivMODE_NDIV;
    if (GsLIOFF == 1)
        lmode = GsLMODE_LOFF;
    else if ((GsLIGNR == 0 && (GsLIGHT_MODE & 1)) || (GsLIGNR == 1 && (GsLMODE & 1)))
        lmode = GsLMODE_FOG;
    else
        lmode = GsLMODE_NORMAL;

    tmd = (struct TMD_STRUCT *)objp->tmd;
    primtop = tmd->primtop;
    primn = tmd->primn;
    vertop = tmd->vertop;
    nortop = tmd->nortop;
    while (primn) {
        int code = ((u_char *)primtop)[3] & 0xfd;
        switch (code) {
        case GPU_COM_F3:
            if (((u_short *)primtop)[1] & GsTMDFlagGRD) {
                GsOUT_PACKET_P = GsFCALL4.f3g[lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 6;
            } else {
                GsOUT_PACKET_P = GsFCALL4.f3[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 4;
            }
            break;
        case GPU_COM_TF3:
            GsOUT_PACKET_P = GsFCALL4.tf3[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 6;
            break;
        case GPU_COM_G3:
            if (((u_short *)primtop)[1] & GsTMDFlagGRD) {
                GsOUT_PACKET_P = GsFCALL4.g3g[lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 7;
            } else {
                GsOUT_PACKET_P = GsFCALL4.g3[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 5;
            }
            break;
        case GPU_COM_TG3:
            GsOUT_PACKET_P = GsFCALL4.tg3[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 7;
            break;
        case GPU_COM_F4:
            if (((u_short *)primtop)[1] & GsTMDFlagGRD) {
                GsOUT_PACKET_P = GsFCALL4.f4g[lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 8;
            } else {
                GsOUT_PACKET_P = GsFCALL4.f4[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 5;
            }
            break;
        case GPU_COM_TF4:
            GsOUT_PACKET_P = GsFCALL4.tf4[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 8;
            break;
        case GPU_COM_G4:
            if (((u_short *)primtop)[1] & GsTMDFlagGRD) {
                GsOUT_PACKET_P = GsFCALL4.g4g[lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 9;
            } else {
                GsOUT_PACKET_P = GsFCALL4.g4[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
                primn -= *(u_short *)primtop;
                primtop += *(u_short *)primtop * 6;
            }
            break;
        case GPU_COM_TG4:
            GsOUT_PACKET_P = GsFCALL4.tg4[ndiv][lmode](primtop, vertop, nortop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 9;
            break;
        case GPU_COM_NF3:
            GsOUT_PACKET_P = GsFCALL4.nf3[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 4;
            break;
        case GPU_COM_NF4:
            GsOUT_PACKET_P = GsFCALL4.nf4[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 4;
            break;
        case GPU_COM_NG3:
            GsOUT_PACKET_P = GsFCALL4.ng3[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 6;
            break;
        case GPU_COM_NTF3:
            GsOUT_PACKET_P = GsFCALL4.ntf3[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 7;
            break;
        case GPU_COM_NTG3:
            GsOUT_PACKET_P = GsFCALL4.ntg3[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 9;
            break;
        case GPU_COM_NG4:
            GsOUT_PACKET_P = GsFCALL4.ng4[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 7;
            break;
        case GPU_COM_NTF4:
            GsOUT_PACKET_P = GsFCALL4.ntf4[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 8;
            break;
        case GPU_COM_NTG4:
            GsOUT_PACKET_P = GsFCALL4.ntg4[ndiv](primtop, vertop, GsOUT_PACKET_P, *(u_short *)primtop, shift, ot, scratch);
            primn -= *(u_short *)primtop;
            primtop += *(u_short *)primtop * 11;
            break;
        default:
            printf("non supported code %x %x\n", code, primtop);
            break;
        }
    }
}
