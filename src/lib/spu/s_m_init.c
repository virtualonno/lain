#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

long SpuInitMalloc(long num, char *top)
{
    SpuMemBlock *rec;

    if (num > 0) {
        rec = (SpuMemBlock *)top;
        rec->addr = SPU_MALLOC_END | SPU_MALLOC_START_ADDR;
        _spu_memList = rec;
        _spu_AllocLastNum = 0;
        _spu_AllocBlockNum = num;
        rec->size = (SPU_RAM_UNITS << _spu_mem_mode_plus) - SPU_MALLOC_START_ADDR;
        return num;
    }
    return 0;
}
