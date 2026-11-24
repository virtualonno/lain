/* Marks n of the streaming ring's sector slots free, from slot start. */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void init_ring_status(int start, u_long n)
{
	u_long i;
	u_long *header;

	for (i = 0; i < n; i++) {
		header = StRingAddr + (i + start) * HEADER_SIZE;
		*header = StFREE;
	}
}
