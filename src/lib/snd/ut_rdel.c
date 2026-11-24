#include <sys/types.h>
#include "snd_vm_int.h"

void SsUtSetReverbDelay(short delay)
{
    _svm_rattr.mask = SPU_REV_DELAYTIME;
    _svm_rattr.delay = delay;
    SpuSetReverbModeParam(&_svm_rattr);
}
