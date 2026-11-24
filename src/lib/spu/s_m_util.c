/*
 * LIBSPU sound RAM allocator overlap checks.
 */
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

/* Return 1 if [addr, end of SPU RAM) overlaps any allocated block, 0 otherwise. */
long _SpuIsInAllocateArea(unsigned long addr)
{
    long i;
    unsigned long start;

    if (!_spu_memList) {
        return 0;
    }

    for (i = 0; ; i++) {
        if (_spu_memList[i].addr & SPU_MALLOC_FREE) {
            continue;
        }
        if (_spu_memList[i].addr & SPU_MALLOC_END) {
            break;
        }
        start = _spu_memList[i].addr & SPU_MALLOC_ADDR;
        if (addr <= start) {
            return 1;
        }
        if (addr < start + _spu_memList[i].size) {
            return 1;
        }
    }
    return 0;
}

/* Same as _SpuIsInAllocateArea, with addr in 8-byte units. */
long _SpuIsInAllocateArea_(unsigned long addr)
{
    long i;
    unsigned long start;

    addr <<= _spu_mem_mode_plus;
    if (!_spu_memList) {
        return 0;
    }

    for (i = 0; ; i++) {
        if (_spu_memList[i].addr & SPU_MALLOC_FREE) {
            continue;
        }
        if (_spu_memList[i].addr & SPU_MALLOC_END) {
            break;
        }
        start = _spu_memList[i].addr & SPU_MALLOC_ADDR;
        if (addr <= start) {
            return 1;
        }
        if (addr < start + _spu_memList[i].size) {
            return 1;
        }
    }
    return 0;
}
