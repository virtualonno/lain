#include "spu_int.h"

#define SPU_RQ_INDEX(reg) ((reg) - SPU_REG_KON_L)

void SpuSetKey(long on_off, unsigned long voice_bit)
{
    unsigned long high_bits;

    voice_bit &= SPU_ALLCH;
    high_bits = voice_bit >> 16;

    switch (on_off) {
    case SPU_ON:
        if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
            _spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_L)] = voice_bit;
            _spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_H)] = high_bits;
            _spu_RQmask |= SPU_EVENT_KEY;
            _spu_RQvoice |= voice_bit;
            if (_spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_L)] & voice_bit) {
                _spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_L)] &= ~voice_bit;
            }
            if (_spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_H)] & high_bits) {
                _spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_H)] &= ~high_bits;
            }
        } else {
            _spu_RXX[SPU_REG_KON_L] = voice_bit;
            _spu_RXX[SPU_REG_KON_H] = high_bits;
            _spu_keystat |= voice_bit;
        }
        break;
    case SPU_OFF:
        if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
            _spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_L)] = voice_bit;
            _spu_RQ[SPU_RQ_INDEX(SPU_REG_KOFF_H)] = high_bits;
            _spu_RQmask |= SPU_EVENT_KEY;
            _spu_RQvoice &= ~voice_bit;
            if (_spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_L)] & voice_bit) {
                _spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_L)] &= ~voice_bit;
            }
            if (_spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_H)] & high_bits) {
                _spu_RQ[SPU_RQ_INDEX(SPU_REG_KON_H)] &= ~high_bits;
            }
        } else {
            _spu_RXX[SPU_REG_KOFF_L] = voice_bit;
            _spu_RXX[SPU_REG_KOFF_H] = high_bits;
            _spu_keystat &= ~voice_bit;
        }
        break;
    }
}
