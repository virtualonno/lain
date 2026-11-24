#include <sys/types.h>
#include <libsnd.h>
#include "psx_hw.h"
#include "snd_vm_int.h"

char SsSetReservedVoice(char voices)
{
    if ((unsigned char)(voices - 1) >= SPU_VOICE_COUNT)
        return -1;

    _SsVmMaxVoice = voices;
    return voices;
}
