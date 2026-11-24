/* Compacts the SPU RAM allocator's table: joins adjacent free blocks and sorts it by address. */
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

void _spu_gcSPU(void)
{
    long i;
    long j;
    unsigned long swap_addr;
    unsigned long swap_size;

    i = 0;
    if (_spu_AllocLastNum >= 0) {
        unsigned long free_flag = SPU_MALLOC_FREE;
        unsigned long empty = SPU_MALLOC_EMPTY;
        unsigned long addr_mask = SPU_MALLOC_ADDR;
        SpuMemBlock *blocks = _spu_memList;
        long last_block = _spu_AllocLastNum;

        do {
            if (blocks[i].addr & free_flag) {
                for (j = i + 1; ; j++) {
                    if (blocks[j].addr == empty) {
                        continue;
                    }
                    break;
                }
                if ((blocks[j].addr & free_flag) &&
                    ((blocks[j].addr & addr_mask) ==
                     ((blocks[i].addr & addr_mask) + blocks[i].size))) {
                    blocks[j].addr = empty;
                    blocks[i].size += blocks[j].size;
                    continue;
                }
            }
            i++;
        } while (i <= last_block);
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].size == 0) {
            _spu_memList[i].addr = SPU_MALLOC_EMPTY;
        }
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & SPU_MALLOC_END) {
            break;
        }
        for (j = i + 1; j <= _spu_AllocLastNum; j++) {
            if (_spu_memList[j].addr & SPU_MALLOC_END) {
                break;
            }
            if ((_spu_memList[j].addr & SPU_MALLOC_ADDR) <
                (_spu_memList[i].addr & SPU_MALLOC_ADDR)) {
                swap_addr = _spu_memList[i].addr;
                swap_size = _spu_memList[i].size;
                _spu_memList[i].addr = _spu_memList[j].addr;
                _spu_memList[i].size = _spu_memList[j].size;
                _spu_memList[j].addr = swap_addr;
                _spu_memList[j].size = swap_size;
            }
        }
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & SPU_MALLOC_END) {
            break;
        }
        if (_spu_memList[i].addr == SPU_MALLOC_EMPTY) {
            _spu_memList[i].addr = _spu_memList[_spu_AllocLastNum].addr;
            _spu_memList[i].size = _spu_memList[_spu_AllocLastNum].size;
            _spu_AllocLastNum = i;
            break;
        }
    }

    for (i = _spu_AllocLastNum - 1; i >= 0; i--) {
        if (_spu_memList[i].addr & SPU_MALLOC_FREE) {
            _spu_memList[i].addr = (_spu_memList[i].addr & SPU_MALLOC_ADDR) | SPU_MALLOC_END;
            _spu_memList[i].size += _spu_memList[_spu_AllocLastNum].size;
            _spu_AllocLastNum = i;
        } else {
            break;
        }
    }
}
