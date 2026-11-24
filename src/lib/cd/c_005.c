/*
 * CD-ROM streaming setup (StSetStream).
 */
#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void (*StFunc1)();
void (*StFunc2)();

void StSetStream(u_long mode, u_long start_frame, u_long end_frame, void (*frame_done)(), void (*stream_end)())
{
    StSetMask(1, start_frame, end_frame);
    StEmu_Addr = 0;
    StFunc1 = frame_done;
    StRgb24 = mode & 1;
    CChannel = 0;
    StCHANNEL = 0;
    Stsector_offset = 0;
    Stframe_no = 0;
    StFunc2 = stream_end;
}
