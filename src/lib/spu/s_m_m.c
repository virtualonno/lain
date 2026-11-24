#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

long SpuMalloc(long size)
{
    long i;
    long found;
    unsigned long rev_size;
    unsigned long block_size;
    unsigned long split_addr;
    unsigned long tail_addr;
    unsigned long tail_size;

    i = 0;
    found = -1;
    if (_spu_rev_reserve_wa == SPU_OFF) {
        rev_size = 0;
    } else {
        rev_size = (SPU_RAM_UNITS - _spu_rev_offsetaddr) << _spu_mem_mode_plus;
    }

    if (size & ~_spu_mem_mode_unitM) {
        size += _spu_mem_mode_unitM;
    }
    size >>= _spu_mem_mode_plus;
    size <<= _spu_mem_mode_plus;

    if (_spu_memList[0].addr & SPU_MALLOC_END) {
        found = 0;
    } else {
        _spu_gcSPU();
        for (; i < _spu_AllocBlockNum; i++) {
            if ((_spu_memList[i].addr & SPU_MALLOC_END) ||
                ((_spu_memList[i].addr & SPU_MALLOC_FREE) && _spu_memList[i].size >= size)) {
                found = i;
                break;
            }
        }
    }

    if (found == -1) {
        return SPU_ERROR;
    }

    if (_spu_memList[found].addr & SPU_MALLOC_END) {
        if (found >= _spu_AllocBlockNum ||
            _spu_memList[found].size - rev_size < size) {
            return SPU_ERROR;
        }
        _spu_AllocLastNum = found + 1;
        _spu_memList[_spu_AllocLastNum].addr = ((_spu_memList[found].addr & SPU_MALLOC_ADDR) + size) | SPU_MALLOC_END;
        _spu_memList[_spu_AllocLastNum].size = _spu_memList[found].size - size;
        _spu_memList[found].size = size;
        _spu_memList[found].addr &= SPU_MALLOC_ADDR;
        _spu_gcSPU();
        return _spu_memList[found].addr;
    }

    block_size = _spu_memList[found].size;
    if (size < block_size && _spu_AllocLastNum < _spu_AllocBlockNum) {
        split_addr = _spu_memList[found].addr + size;
        tail_addr = _spu_memList[_spu_AllocLastNum].addr;
        tail_size = _spu_memList[_spu_AllocLastNum].size;
        _spu_memList[_spu_AllocLastNum].addr = split_addr | SPU_MALLOC_FREE;
        _spu_memList[_spu_AllocLastNum].size = block_size - size;
        _spu_memList[_spu_AllocLastNum + 1].addr = tail_addr;
        _spu_memList[_spu_AllocLastNum + 1].size = tail_size;
        _spu_AllocLastNum++;
    }

    _spu_memList[found].size = size;
    _spu_memList[found].addr &= SPU_MALLOC_ADDR;
    _spu_gcSPU();
    return _spu_memList[found].addr;
}
