#include <sys/types.h>
#include "snd_vm_int.h"

#include "vm_prog.h"
#include "vm/vm_vsu.h"

int _SsVmSetProgVol(short vab_id, short prog, unsigned char volume)
{
    if (_SsVmVSetUp(vab_id, prog) != 0) {
        return -1;
    }
    _svm_pg[prog].mvol = volume;
    return _svm_pg[prog].mvol;
}

int _SsVmGetProgVol(short vab_id, short prog)
{
    if (_SsVmVSetUp(vab_id, prog) != 0) {
        return -1;
    }
    return _svm_pg[prog].mvol;
}

int _SsVmSetProgPan(short vab_id, short prog, unsigned char pan)
{
    if (_SsVmVSetUp(vab_id, prog) != 0) {
        return -1;
    }
    _svm_pg[prog].mpan = pan;
    return _svm_pg[prog].mpan;
}

int _SsVmGetProgPan(short vab_id, short prog)
{
    if (_SsVmVSetUp(vab_id, prog) != 0) {
        return -1;
    }
    return _svm_pg[prog].mpan;
}
