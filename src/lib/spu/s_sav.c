/*
 * Turns the given voices on or off, or replaces the whole mask, in one SPU voice-bit register
 * pair or in its _spu_RQ mirror while events queue. The _spu_RQmask bit raised is the pair's
 * event flag: SPU_EVENT_KEY for KOFF, then _PITCHLFO, _NOISE and _REVERB for PMON, NON and EON.
 */
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

unsigned long _SpuSetAnyVoice(long on_off, unsigned long voice_bit, long reg_lo, long reg_hi)
{
    unsigned long voices;

    if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
        voices = ((_spu_RQ[reg_hi - SPU_REG_KON_L] & 0xff) << 16) | _spu_RQ[reg_lo - SPU_REG_KON_L];
    } else {
        voices = ((_spu_RXX[reg_hi] & 0xff) << 16) | _spu_RXX[reg_lo];
    }

    switch (on_off) {
    case SPU_ON:
        if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
            _spu_RQ[reg_lo - SPU_REG_KON_L] |= voice_bit;
            _spu_RQ[reg_hi - SPU_REG_KON_L] |= (voice_bit >> 16) & 0xff;
            _spu_RQmask |= 1 << ((reg_lo - SPU_REG_KOFF_L) >> 1);
        } else {
            _spu_RXX[reg_lo] |= voice_bit;
            _spu_RXX[reg_hi] |= (voice_bit >> 16) & 0xff;
        }
        voices |= voice_bit & SPU_ALLCH;
        break;
    case SPU_OFF:
        if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
            _spu_RQ[reg_lo - SPU_REG_KON_L] &= ~voice_bit;
            _spu_RQ[reg_hi - SPU_REG_KON_L] &= ~((voice_bit >> 16) & 0xff);
            _spu_RQmask |= 1 << ((reg_lo - SPU_REG_KOFF_L) >> 1);
        } else {
            _spu_RXX[reg_lo] &= ~voice_bit;
            _spu_RXX[reg_hi] &= ~((voice_bit >> 16) & 0xff);
        }
        voices &= ~(voice_bit & SPU_ALLCH);
        break;
    case SPU_BIT:
        if (_spu_env & SPU_ENV_EVENT_QUEUEING) {
            _spu_RQ[reg_lo - SPU_REG_KON_L] = voice_bit;
            _spu_RQ[reg_hi - SPU_REG_KON_L] = (voice_bit >> 16) & 0xff;
            _spu_RQmask |= 1 << ((reg_lo - SPU_REG_KOFF_L) >> 1);
        } else {
            _spu_RXX[reg_lo] = voice_bit;
            _spu_RXX[reg_hi] = (voice_bit >> 16) & 0xff;
        }
        voices = voice_bit & SPU_ALLCH;
        break;
    }

    return voices & SPU_ALLCH;
}
