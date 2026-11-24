#include <sys/types.h>
#include "snd_vm_int.h"

#include "vm/vm_vsu.h"

short SsUtGetVagAtr(short vab, short prog, short tone, VagAtr *attr)
{
    short index;

    if (_svm_vab_used[vab] == SVM_VAB_ACTIVE) {
        _SsVmVSetUp(vab, prog);
        index = tone + (signed char)_svm_cur.prog_index * VAB_TONES_PER_PROG;
        attr->prior = _svm_tn[index].prior;
        attr->mode = _svm_tn[index].mode;
        attr->vol = _svm_tn[index].vol;
        attr->pan = _svm_tn[index].pan;
        attr->center = _svm_tn[index].center;
        attr->shift = _svm_tn[index].shift;
        attr->max = _svm_tn[index].max;
        attr->min = _svm_tn[index].min;
        attr->vibW = _svm_tn[index].vibW;
        attr->vibT = _svm_tn[index].vibT;
        attr->porW = _svm_tn[index].porW;
        attr->porT = _svm_tn[index].porT;
        attr->pbmin = _svm_tn[index].pbmin;
        attr->pbmax = _svm_tn[index].pbmax;
        attr->adsr1 = _svm_tn[index].adsr1;
        attr->adsr2 = _svm_tn[index].adsr2;
        attr->prog = _svm_tn[index].prog;
        attr->vag = _svm_tn[index].vag;
        return 0;
    }
    return -1;
}
