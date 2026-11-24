/*
 * CD-ROM streaming ring consumer (StGetNext): returns and locks the next
 * complete frame in the ring buffer (0: ready, 1: not ready).
 */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

u_long StGetNext(u_long **addr, u_long **header)
{
	volatile u_short *ring_header;

	ring_header = (volatile u_short *)(StRingAddr + StRingIdx3 * HEADER_SIZE);
	if (ring_header[StSTATUS] == StREWIND) {
		StRingIdx3 = 0;
		if (StEndFrame)
			ring_header[StSTATUS] = StFREE;
		ring_header = (volatile u_short *)(StRingAddr + StRingIdx3 * HEADER_SIZE);
	}
	if (ring_header[StSTATUS] != StCOMPLETE)
		return 1;
	ring_header[StSTATUS] = StLOCK;
	*addr = StRingAddr + StRingSize * HEADER_SIZE + StRingIdx3 * (SECTOR_SIZE - HEADER_SIZE);
	*header = (u_long *)ring_header;
	return 0;
}
