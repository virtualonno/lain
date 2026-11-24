#include "spu_int.h"

long SpuSetNoiseClock(long clock)
{
    long rate;

    if (clock < 0) {
        rate = 0;
    } else if (clock > SPUCNT_NOISE_CLK_MASK) {
        rate = SPUCNT_NOISE_CLK_MASK;
    } else {
        rate = clock;
    }
    _spu_RXX[SPU_REG_SPUCNT] = (_spu_RXX[SPU_REG_SPUCNT] & ~SPUCNT_NOISE_CLK_BITS) |
                               ((rate & SPUCNT_NOISE_CLK_MASK) << SPUCNT_NOISE_CLK_SHIFT);
    return rate;
}
