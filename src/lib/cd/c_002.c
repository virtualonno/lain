/* Reset the CD-ROM streaming ring buffer state and sector status headers. */

#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void StClearRing(void)
{
    StRingIdx3 = 0;
    StRingIdx2 = 0;
    StRingIdx1 = 0;
    StFinalSector = 0;
    init_ring_status(0, StRingSize);
    StCdIntrFlag = 0;
    Stsector_offset = 0;
    Stframe_no = 0;
}
