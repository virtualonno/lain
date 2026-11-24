#include <sys/types.h>
#include <libgte.h>
#include "libgs_int.h"

void gte_init(void)
{
    InitGeom();
    SetFarColor(0, 0, 0);
    SetGeomOffset(0, 0);
    GsORGOFSY = 0;
    GsORGOFSX = 0;
}
