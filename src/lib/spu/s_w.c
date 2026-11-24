#include <stddef.h>
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

#define SPU_MAX_TRANSFER_SIZE (SPU_RAM_SIZE - SPU_MALLOC_START_ADDR)

unsigned long SpuWrite(unsigned char *addr, unsigned long size)
{
    if (size > SPU_MAX_TRANSFER_SIZE) {
        size = SPU_MAX_TRANSFER_SIZE;
    }
    _spu_Fw(addr, size);
    if (_spu_transferCallback == NULL) {
        _spu_inTransfer = 0;
    }
    return size;
}
