#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

void GsSetOrign(long x, long y)
{
    POSITION.offx = x;
    POSITION.offy = y;
}
