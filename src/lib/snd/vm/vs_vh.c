#include "snd_vm_int.h"
#include "spu_int.h"
#include "vs_vh.h"

#define VAB_FORM_MAGIC 0x564142

static long VabFixedAddress(unsigned long size, unsigned long sbaddr, short vabid);

short SsVabOpenHeadSticky(unsigned char *addr, short vabid, unsigned long sbaddr)
{
    return _SsVabOpenHeadWithMode(addr, vabid, VabFixedAddress, sbaddr);
}

short SsVabFakeHead(unsigned char *addr, short vabid, unsigned long sbaddr)
{
    return _SsVabOpenHeadWithMode(addr, vabid, VabFixedAddress, sbaddr);
}

static long VabFixedAddress(unsigned long size, unsigned long sbaddr, short vabid)
{
    return sbaddr;
}

short _SsVabOpenHeadWithMode(unsigned char *addr, short vabid, VabAlloc alloc, unsigned long sbaddr)
{
    unsigned long vag_size[VAB_MAX_VAGS];
    int i;
    short id;
    unsigned char *p;
    VabHdr *vh;
    ProgAtr *pg;
    unsigned char vs;
    unsigned long total;
    long start_addr;
    unsigned short size;

    id = SVM_VAB_MAX;
    if (_spu_getInTransfer() == 1) {
        return -1;
    }
    _spu_setInTransfer(1);
    if (vabid >= SVM_VAB_MAX) {
        _spu_setInTransfer(0);
        return -1;
    }
    if (vabid == -1) {
        for (i = 0; i < SVM_VAB_MAX; i++) {
            if (_svm_vab_used[i] == SVM_VAB_FREE) {
                _svm_vab_used[i] = SVM_VAB_ACTIVE;
                id = i;
                _svm_vab_count++;
                break;
            }
        }
    } else if (_svm_vab_used[vabid] == SVM_VAB_FREE) {
        _svm_vab_used[vabid] = SVM_VAB_ACTIVE;
        id = vabid;
        _svm_vab_count++;
    }
    if (id >= SVM_VAB_MAX) {
        _spu_setInTransfer(0);
        return -1;
    }

    p = addr;
    _svm_vab_vh[id] = (VabHdr *)p;
    p += sizeof(VabHdr);
    vh = (VabHdr *)addr;
    if (((unsigned long)vh->form >> 8) != VAB_FORM_MAGIC) {
        _svm_vab_used[id] = SVM_VAB_FREE;
        _spu_setInTransfer(0);
        _svm_vab_count--;
        return -1;
    }

    if ((vh->form & 0xff) == 'p' && vh->ver >= 5) {
        kMaxPrograms = 128;
    } else {
        kMaxPrograms = 64;
    }
    if (vh->ps > kMaxPrograms) {
        _svm_vab_used[id] = SVM_VAB_FREE;
        _spu_setInTransfer(0);
        _svm_vab_count--;
        return -1;
    }

    _svm_vab_pg[id] = (ProgAtr *)p;
    pg = (ProgAtr *)p;
    p += sizeof(ProgAtr) * kMaxPrograms;
    for (i = 0, total = 0; i < kMaxPrograms; i++) {
        pg[i].reserved1 = total;
        if (pg[i].tones > 0) {
            total++;
        }
    }

    _svm_vab_tn[id] = (VagAtr *)p;
    total = 0;
    vs = vh->vs;
    p += vh->ps * VAB_TONES_PER_PROG * sizeof(VagAtr);
    for (i = 0; i < VAB_MAX_VAGS; i++) {
        if (i <= vs) {
            size = *(unsigned short *)p;
            if (vh->ver >= 5) {
                vag_size[i] = size << 3;
            } else {
                vag_size[i] = size << 2;
            }
            total += vag_size[i];
        }
        p += sizeof(unsigned short);
    }

    start_addr = alloc(total, sbaddr, id);
    if (start_addr == -1) {
        return -1;
    }
    if (start_addr + total > SPU_RAM_SIZE) {
        _svm_vab_used[id] = SVM_VAB_FREE;
        _spu_setInTransfer(0);
        _svm_vab_count--;
        return -1;
    }

    _svm_vab_start[id] = start_addr;
    total = 0;
    for (i = 0; i <= vs; i++) {
        total += vag_size[i];
        if ((i & 1) == 0) {
            ((unsigned short *)&pg[i / 2].reserved2)[0] = (start_addr + total) >> 3;
        } else {
            ((unsigned short *)&pg[i / 2].reserved2)[1] = (start_addr + total) >> 3;
        }
    }
    _svm_vab_total[id] = total;
    _svm_vab_used[id] = SVM_VAB_HEAD_OPEN;
    return id;
}
