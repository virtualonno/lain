#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

void SsUtReverbOn(void)
{
    SpuSetReverb(SPU_ON);
}

