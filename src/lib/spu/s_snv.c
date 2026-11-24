#include <sys/types.h>
#include <libspu.h>
#include "spu_int.h"

unsigned long SpuSetNoiseVoice(long on_off, unsigned long voice_bit)
{
    return _SpuSetAnyVoice(on_off, voice_bit, SPU_REG_NON_L, SPU_REG_NON_H);
}
