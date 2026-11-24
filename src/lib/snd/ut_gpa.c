#include <sys/types.h>
#include "snd_vm_int.h"

#include "vm/vm_vsu.h"

short SsUtGetProgAtr(short vab, short prog, ProgAtr *attr)
{
    if (_svm_vab_used[vab] == SVM_VAB_ACTIVE) {
        _SsVmVSetUp(vab, prog);
        attr->tones = _svm_pg[prog].tones;
        attr->mvol = _svm_pg[prog].mvol;
        attr->prior = _svm_pg[prog].prior;
        attr->mode = _svm_pg[prog].mode;
        attr->mpan = _svm_pg[prog].mpan;
        attr->attr = _svm_pg[prog].attr;
        return 0;
    }
    return -1;
}
