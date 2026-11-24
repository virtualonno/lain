#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

long SpuSetTransferMode(long mode)
{
    long trans_mode;

    switch (mode) {
    case SPU_TRANSFER_BY_DMA:
        trans_mode = SPU_TRANSFER_BY_DMA;
        break;
    case SPU_TRANSFER_BY_IO:
        trans_mode = SPU_TRANSFER_BY_IO;
        break;
    default:
        trans_mode = SPU_TRANSFER_BY_DMA;
        break;
    }
    _spu_trans_mode = mode;
    _spu_transMode = trans_mode;
    return trans_mode;
}
