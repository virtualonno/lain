/* Set up a newly allocated voice: its envelope tracking, and its buffered
 * start-address and ADSR registers. */
#include <sys/types.h>
#include <libsnd.h>

#include "snd_vm_int.h"
#include "vm_aloc2.h"
#include "vm_f.h"

#define ADSR2_RELEASE_MASK 0x001f
#define ADSR2_OTHER_BITS   0xffe0

void _SsVmDoAllocate(void)
{
    int i;
    short sreg_offset;
    int tone_index;
    unsigned short adsr2;
    short release;

    sreg_offset = (unsigned short)_svm_cur.voice * SPU_VREG_COUNT;
    _svm_voice[_svm_cur.voice].envx = SVM_ENVX_MAX;

    for (i = 0; i < SVM_ENVX_HIST_MAX; i++) {
        _svm_envx_hist[i] &= ~(1 << _svm_cur.voice);
    }

    /* _SsVabOpenHeadWithMode keeps 1-based VAG n's SPU address, in 8-byte units, in
     * half (n - 1) & 1 of _svm_pg[(n - 1) / 2].reserved2. */
    if ((_svm_cur.vag_idx & 1) > 0) {
        _svm_sreg_buf[sreg_offset + SPU_VREG_START_ADDR] =
            _svm_pg[(_svm_cur.vag_idx - 1) / 2].reserved2 & 0xffff;
        _svm_sreg_dirty[_svm_cur.voice] |= SREG_DIRTY_START_ADDR;
    } else {
        _svm_sreg_buf[sreg_offset + SPU_VREG_START_ADDR] =
            _svm_pg[(_svm_cur.vag_idx - 1) / 2].reserved2 >> 16;
        _svm_sreg_dirty[_svm_cur.voice] |= SREG_DIRTY_START_ADDR;
    }

    tone_index = _svm_cur.prog_index * VAB_TONES_PER_PROG + _svm_cur.tone;
    _svm_sreg_buf[sreg_offset + SPU_VREG_ADSR1] = _svm_tn[tone_index].adsr1;
    adsr2 = _svm_tn[tone_index].adsr2;
    release = (adsr2 & ADSR2_RELEASE_MASK) + _svm_damper;
    adsr2 &= ADSR2_OTHER_BITS;
    if (release > ADSR2_RELEASE_MASK) {
        release = ADSR2_RELEASE_MASK;
    }
    release |= adsr2;
    _svm_sreg_buf[sreg_offset + SPU_VREG_ADSR2] = release;
    _svm_sreg_dirty[_svm_cur.voice] |= SREG_DIRTY_ADSR;
}
