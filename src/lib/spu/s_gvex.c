#include "spu_int.h"

void SpuGetVoiceEnvelope(int voice, short *envx)
{
    *envx = _spu_RXX[voice * SPU_VREG_COUNT + SPU_VREG_ENVX];
}
