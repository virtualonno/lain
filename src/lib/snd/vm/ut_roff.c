#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

void SsUtReverbOff(void)
{
    SpuSetReverb(SPU_OFF);
}
