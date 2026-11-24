/*
 * VAB body transfer to SPU sound RAM.
 */

#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

#include "snd_vm_int.h"
#include "spu_int.h"

short SsVabTransBody(unsigned char *addr, short vabid)
{
    unsigned long start;

    /* vabid 16 passes and reads one past each 16-entry table */
    if (vabid >= 0 && vabid <= SVM_VAB_MAX) {
        if (_svm_vab_used[vabid] == SVM_VAB_HEAD_OPEN) {
            start = _svm_vab_start[vabid];
            SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
            if (SpuSetTransferStartAddr(start) != 0) {
                SpuWrite(addr, _svm_vab_total[vabid]);
                _svm_vab_used[vabid] = SVM_VAB_ACTIVE;
                return vabid;
            }
        }
    }
    _spu_setInTransfer(0);
    return -1;
}
