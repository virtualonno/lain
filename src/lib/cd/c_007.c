/* Release a locked frame's sectors and advance the stream's read index. */
#include <sys/types.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

u_long StFreeRing(u_long *base)
{
    int sector_index, i;
    short sector_count;
    StHEADER *header;

    sector_index = (base - (StRingAddr + StRingSize * HEADER_SIZE)) / (SECTOR_SIZE - HEADER_SIZE);
    header = (StHEADER *)StRingAddr + sector_index;
    sector_count = header->nSectors;
    if (((short *)header)[StSTATUS] != StLOCK)
        return 1;

    for (i = 0; i < sector_count; i++) {
        StHEADER *slot = (StHEADER *)StRingAddr + (i + sector_index);
        ((short *)slot)[StSTATUS] = StFREE;
    }
    StRingIdx3 = i + sector_index;
    return 0;
}
