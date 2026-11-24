#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

int GsGetActiveBuff(void)
{
    return PSDIDX;
}

void GsSetDrawBuffOffset(void)
{
    int ofx, ofy;

    if (PSDGPU) {
        GsORGOFSY = 0;
        GsORGOFSX = 0;
        GsDRAWENV.ofs[0] = POSITION.offx + PSDOFSX[PSDIDX];
        GsDRAWENV.ofs[1] = POSITION.offy + PSDOFSY[PSDIDX];
        PutDrawEnv(&GsDRAWENV);
    } else {
        ofx = POSITION.offx + PSDOFSX[!PSDIDX];
        ofy = POSITION.offy + PSDOFSY[!PSDIDX];
        SetGeomOffset(ofx, ofy);
        GsORGOFSX = ofx;
        GsORGOFSY = ofy;
    }
}
