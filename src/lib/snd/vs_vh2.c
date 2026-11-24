#include "snd_vm_int.h"
#include "spu_int.h"
#include "vm/vs_vh.h"

static long VabAllocate(unsigned long size, unsigned long sbaddr, short vabid);

short SsVabOpenHead(unsigned char *addr, short vabid)
{
    return _SsVabOpenHeadWithMode(addr, vabid, VabAllocate, 0);
}

static long VabAllocate(unsigned long size, unsigned long sbaddr, short vabid)
{
    long start_addr;

    start_addr = SpuMalloc(size);
    if (start_addr == -1) {
        _svm_vab_used[vabid] = SVM_VAB_FREE;
        _spu_setInTransfer(0);
        _svm_vab_count--;
        return -1;
    }
    return start_addr;
}
