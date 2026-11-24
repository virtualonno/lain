#include <stdio.h>
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

long SpuSetIRQ(long on_off)
{
    unsigned long count;

    if (on_off == SPU_OFF || on_off == SPU_RESET) {
        _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_IRQ_ENABLE;
        count = 0;
        while (_spu_RXX[SPU_REG_SPUCNT] & SPUCNT_IRQ_ENABLE) {
            if (++count > SPU_WAIT_TIMEOUT) {
                printf("SPU:T/O [%s]\n", "wait (IRQ/ON)");
                return SPU_ERROR;
            }
        }
    }
    if (on_off == SPU_ON || on_off == SPU_RESET) {
        _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_IRQ_ENABLE;
        count = 0;
        while (!(_spu_RXX[SPU_REG_SPUCNT] & SPUCNT_IRQ_ENABLE)) {
            if (++count > SPU_WAIT_TIMEOUT) {
                printf("SPU:T/O [%s]\n", "wait (IRQ/OFF)");
                return SPU_ERROR;
            }
        }
    }
    return on_off;
}
