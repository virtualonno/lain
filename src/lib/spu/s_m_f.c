#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

void SpuFree(unsigned long addr)
{
    long i;

    for (i = 0; i < _spu_AllocBlockNum; i++) {
        if (_spu_memList[i].addr & SPU_MALLOC_END) {
            break;
        }
        if (_spu_memList[i].addr == addr) {
            _spu_memList[i].addr = addr | SPU_MALLOC_FREE;
            break;
        }
    }
    _spu_gcSPU();
}
