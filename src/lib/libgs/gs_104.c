#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

long HWD0, VWD0;
long GsCLIP3near, GsCLIP3far;
int GsLIGHT_MODE;

void GsInit3D(void)
{
    POSITION.offx = HWD0 / 2;
    POSITION.offy = VWD0 / 2;
    GsSetDrawBuffOffset();
    GsCLIP3near = 10;
    GsLIGHT_MODE = 0;
    GsCLIP3far = 0x3fff;
}
