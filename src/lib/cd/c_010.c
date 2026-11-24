/* Set the stream's frame window: wait for frame start (mask 1), drop frames from end on. */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void StSetMask(u_long mask, u_long start, u_long end)
{
    StSTART_FLAG = mask;
    StStartFrame = start;
    StEndFrame = end;
}
