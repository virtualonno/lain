/* CD-ROM stream ring frame-completion callback and back-location query. */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

static CdlLOC completed_frame_loc;
static u_long completed_frame_no;

void data_ready_callback(void)
{
	StHEADER *header;

	header = (StHEADER *)(StRingAddr + StRingIdx2 * HEADER_SIZE);
	((u_short *)header)[StSTATUS] = StCOMPLETE;
	completed_frame_loc = header->loc;
	completed_frame_no = header->frameCount;
	StRingIdx2 = StRingIdx1;
	if (StFunc1)
		(*StFunc1)();
	StFinalSector = 0;
}

int StGetBackloc(CdlLOC *loc)
{
	if (StMode != ST_MODE_2340)
		return -1;
	CdIntToPos(CdPosToInt(&completed_frame_loc) + 1, loc);
	return completed_frame_no;
}
