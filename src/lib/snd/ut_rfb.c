#include <sys/types.h>
#include "snd_vm_int.h"

void SsUtSetReverbFeedback(short feedback)
{
    _svm_rattr.mask = SPU_REV_FEEDBACK;
    _svm_rattr.feedback = feedback;
    SpuSetReverbModeParam(&_svm_rattr);
}
