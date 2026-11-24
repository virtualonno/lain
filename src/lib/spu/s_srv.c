#include "spu_int.h"

unsigned long SpuSetReverbVoice(long on_off, unsigned long voice_bit)
{
    return _SpuSetAnyVoice(on_off, voice_bit, SPU_REG_EON_L, SPU_REG_EON_H);
}
