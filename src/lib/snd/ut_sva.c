#include <sys/types.h>
#include "snd_vm_int.h"
#include "vm/vm_vsu.h"

short SsUtSetVagAtr(short vab, short prog, short tone, VagAtr *attr)
{
    short index;

    if (_svm_vab_used[vab] == SVM_VAB_ACTIVE) {
        _SsVmVSetUp(vab, prog);
        index = tone + (signed char)_svm_cur.prog_index * VAB_TONES_PER_PROG;
        _svm_tn[index].prior = attr->prior;
        _svm_tn[index].mode = attr->mode;
        _svm_tn[index].vol = attr->vol;
        _svm_tn[index].pan = attr->pan;
        _svm_tn[index].center = attr->center;
        _svm_tn[index].shift = attr->shift;
        _svm_tn[index].max = attr->max;
        _svm_tn[index].min = attr->min;
        _svm_tn[index].vibW = attr->vibW;
        _svm_tn[index].vibT = attr->vibT;
        _svm_tn[index].porW = attr->porW;
        _svm_tn[index].porT = attr->porT;
        _svm_tn[index].pbmin = attr->pbmin;
        _svm_tn[index].pbmax = attr->pbmax;
        _svm_tn[index].adsr1 = attr->adsr1;
        _svm_tn[index].adsr2 = attr->adsr2;
        _svm_tn[index].prog = attr->prog;
        _svm_tn[index].vag = attr->vag;
        return 0;
    }
    return -1;
}
