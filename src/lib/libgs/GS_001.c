#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "libgs_int.h"

RECT CLIP2;
DISPENV GsDISPENV;
DRAWENV GsDRAWENV;
short GsORGOFSX, GsORGOFSY;
_GsPOSITION POSITION;
short PSDBASEX[2], PSDBASEY[2];
u_long PSDCNT;
short PSDGPU, PSDIDX;
short PSDOFSX[2], PSDOFSY[2];

/* 16 bytes: "Ps", a byte that differs by library, then five more; nothing reads them */
static unsigned char gs_id[16] = { 0x50, 0x73, 0x08, 0x25, 0x9b, 0x53, 0x43, 0x00 };
static TILE clearPacket[2];

static void InitEnvironments(u_short w, u_short h, u_short mode, u_short dither, u_short rgb24);
static void InitGeometry(u_short w, u_short h);

void GsInitGraph(u_short w, u_short h, u_short mode, u_short dither, u_short rgb24)
{
    InitEnvironments(w, h, mode, dither, rgb24);
    gte_init();
    PSDIDX = 0;
    InitGeometry(w, h);
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}

static void InitEnvironments(u_short w, u_short h, u_short mode, u_short dither, u_short rgb24)
{
    ResetGraph(((mode >> 4) & 3) == 3 ? 3 : 0);
    GsDRAWENV.ofs[0] = GsDRAWENV.ofs[1] = 0;
    GsDRAWENV.tw.x = GsDRAWENV.tw.y = GsDRAWENV.tw.w = GsDRAWENV.tw.h = 0;
    GsDRAWENV.tpage = 0;
    GsDRAWENV.dtd = dither;
    GsDRAWENV.dfe = 0;
    GsDRAWENV.isbg = 0;
    PutDrawEnv(&GsDRAWENV);

    setRECT(&GsDISPENV.disp, 0, 0, w, h);
    setRECT(&GsDISPENV.screen, 0, 0, 0, 0);
    if (GetVideoMode() == MODE_PAL) {
        GsDISPENV.screen.y = 24;
        GsDISPENV.pad0 = 1;
    }
    GsDISPENV.isinter = mode & 1;
    PSDGPU = mode & 4;
    GsDISPENV.isrgb24 = rgb24;
    PutDispEnv(&GsDISPENV);
}

void GsInitGraph2(u_short w, u_short h, u_short mode, u_short dither, u_short rgb24)
{
    GsDRAWENV.tpage = 0;
    GsDRAWENV.dtd = dither;
    GsDRAWENV.dfe = 0;
    GsDRAWENV.isbg = 0;
    GsDISPENV.disp.w = w;
    GsDISPENV.disp.h = h;
    GsDISPENV.isinter = mode & 1;
    PSDGPU = mode & 4;
    GsDISPENV.isrgb24 = rgb24;
    InitGeometry(w, h);
}

static void InitGeometry(u_short w, u_short h)
{
    long aspect;
    RECT *clip = &CLIP2;

    {
        /* kept: the original stores both and reads them back from memory */
        extern volatile long HWD0, VWD0;

        HWD0 = w;
        VWD0 = h;
        aspect = (VWD0 << 14) / HWD0;
    }

    GsIDMATRIX.m[2][2] = GsIDMATRIX.m[1][1] = GsIDMATRIX.m[0][0] = (
        GsIDMATRIX.m[0][1] = GsIDMATRIX.m[0][2] = 0,
        GsIDMATRIX.m[1][0] = GsIDMATRIX.m[1][2] = 0,
        GsIDMATRIX.m[2][0] = GsIDMATRIX.m[2][1] = 0,
        GsIDMATRIX.t[0] = GsIDMATRIX.t[1] = GsIDMATRIX.t[2] = 0,
        ONE);
    GsIDMATRIX2 = GsIDMATRIX;
    GsLIGHTWSMATRIX = GsIDMATRIX;
    GsLIGHTWSMATRIX.m[0][0] = GsLIGHTWSMATRIX.m[1][1] = GsLIGHTWSMATRIX.m[2][2] = 0;
    _LC = GsLIGHTWSMATRIX;

    PSDBASEX[0] = 0;
    PSDBASEX[1] = 0;
    PSDBASEY[0] = 0;
    PSDBASEY[1] = 0;
    POSITION.offx = POSITION.offy = 0;
    clip->y = 0;
    GsIDMATRIX2.m[1][1] = aspect / 3;
    clip->x = 0;
    setlen(&clearPacket[0], 3);
    setcode(&clearPacket[0], 2);
    setlen(&clearPacket[1], 3);
    setcode(&clearPacket[1], 2);
    /* kept: the original reads only the low halfword */
    clip->w = *(u_short *)&HWD0;
    /* kept: the original reads only the low halfword */
    clip->h = *(u_short *)&VWD0;
    PSDCNT = 1;
}

void GsSortClear(u_char r, u_char g, u_char b, GsOT *ot)
{
    int buf;
    short x, y, h;
    clearPacket[PSDIDX].r0 = r;
    clearPacket[PSDIDX].g0 = g;
    clearPacket[PSDIDX].b0 = b;
    buf = PSDIDX;
    x = PSDOFSX[buf];
    /* kept: the original reads only the low halfword */
    h = *(u_short *)&VWD0;
    clearPacket[buf].x0 = x;
    y = PSDOFSY[buf];
    clearPacket[buf].h = h;
    clearPacket[buf].y0 = y;
    if (GsDISPENV.isrgb24)
        clearPacket[buf].w = HWD0 * 3 / 2;
    else
        /* kept: the original reads only the low halfword */
        clearPacket[buf].w = *(u_short *)&HWD0;
    AddPrim(ot->tag, &clearPacket[PSDIDX]);
}
