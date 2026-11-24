/* Register the CD-ROM streaming ring buffer and reset its state (StSetRing). */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void StSetRing(u_long *ring_addr, u_long ring_size)
{
    StRingAddr = ring_addr;
    StRingSize = ring_size;
    StClearRing();
}
