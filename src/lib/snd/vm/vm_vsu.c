/* Voice-manager VAB and program setup. */

#include <sys/types.h>
#include "snd_vm_int.h"
#include "vm_vsu.h"

int _SsVmVSetUp(short vab_id, short prog)
{
    if ((unsigned short)vab_id >= SVM_VAB_MAX ||
        _svm_vab_used[vab_id] != SVM_VAB_ACTIVE || prog >= kMaxPrograms) {
        return -1;
    }

    _svm_vh = _svm_vab_vh[vab_id];
    _svm_pg = _svm_vab_pg[vab_id];
    _svm_tn = _svm_vab_tn[vab_id];
    _svm_cur.vab_id = vab_id;
    _svm_cur.prog = prog;
    _svm_cur.prog_index = _svm_pg[prog].reserved1;
    return 0;
}
