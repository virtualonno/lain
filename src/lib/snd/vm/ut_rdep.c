#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

#include "snd_vm_int.h"

void SsUtSetReverbDepth(short ldepth, short rdepth)
{
    _svm_rattr.mask = SPU_REV_DEPTHL | SPU_REV_DEPTHR;
    _svm_rattr.depth.left = (ldepth * 0x7fff) / 127;
    _svm_rattr.depth.right = (rdepth * 0x7fff) / 127;
    SpuSetReverbModeParam(&_svm_rattr);
}
