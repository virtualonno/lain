#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

#include "snd_vm_int.h"

short SsUtSetReverbType(short type)
{
    int clear = 0;
    short rev_type = type;

    if (type < 0) {
        clear = 1;
        rev_type = -type;
    }

    if (rev_type >= SS_REV_TYPE_OFF && rev_type <= SS_REV_TYPE_PIPE) {
        _svm_rattr.mask = SPU_REV_MODE;
        if (clear) {
            _svm_rattr.mode = rev_type | SPU_REV_MODE_CLEAR_WA;
        } else {
            _svm_rattr.mode = rev_type;
        }
        if (rev_type == SS_REV_TYPE_OFF) {
            SpuSetReverb(SPU_OFF);
        }
        SpuSetReverbModeParam(&_svm_rattr);
        return rev_type;
    }

    return -1;
}

short SsUtGetReverbType(void)
{
    return _svm_rattr.mode;
}
