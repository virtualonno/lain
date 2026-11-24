#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

long SpuSetReverb(long on_off)
{
    unsigned short spucnt;

    switch (on_off) {
    case SPU_OFF:
        _spu_rev_flag = SPU_OFF;
        spucnt = _spu_RXX[SPU_REG_SPUCNT];
        spucnt &= ~SPUCNT_REVERB_ENABLE;
        _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        break;
    case SPU_ON:
        if (_spu_rev_reserve_wa != SPU_ON &&
            _SpuIsInAllocateArea_(_spu_rev_offsetaddr)) {
            _spu_rev_flag = SPU_OFF;
            spucnt = _spu_RXX[SPU_REG_SPUCNT];
            spucnt &= ~SPUCNT_REVERB_ENABLE;
            _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        } else {
            _spu_rev_flag = on_off;
            spucnt = _spu_RXX[SPU_REG_SPUCNT];
            spucnt |= SPUCNT_REVERB_ENABLE;
            _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        }
        break;
    }
    return _spu_rev_flag;
}
