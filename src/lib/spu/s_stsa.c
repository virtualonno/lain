#include "spu_int.h"

#define SPU_TRANS_ADDR_MAX 0x7fff8

unsigned long SpuSetTransferStartAddr(unsigned long addr)
{
    unsigned long result;

    if (addr >= SPU_MALLOC_START_ADDR && addr <= SPU_TRANS_ADDR_MAX) {
        _spu_tsa = _spu_FsetRXXa(SPU_CHECK, addr);
        result = (unsigned long)_spu_tsa << _spu_mem_mode_plus;
    } else {
        result = 0;
    }
    return result;
}
