#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

#include "snd_vm_int.h"

void SsVabClose(short vabid)
{
    if (vabid >= 0 && vabid < SVM_VAB_MAX) {
        if (_svm_vab_used[vabid] == SVM_VAB_ACTIVE) {
            SpuFree(_svm_vab_start[vabid]);
            _svm_vab_used[vabid] = SVM_VAB_FREE;
            _svm_vab_count--;
        }
    }
}
