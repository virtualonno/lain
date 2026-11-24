#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

#define SPU_IRQ_ADDR_MAX (SPU_RAM_SIZE - 8)

unsigned long SpuSetIRQAddr(unsigned long addr)
{
    unsigned long result;

    if (addr <= SPU_IRQ_ADDR_MAX) {
        result = _spu_FsetRXXa(SPU_REG_IRQ_ADDR, addr);
    } else {
        result = 0;
    }
    return result;
}
