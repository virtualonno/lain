/* Queue key-on and update the buffered SPU registers for the current tone. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_f.h"
#include "vm_nowon.h"

void _SsVmKeyOnNow(short tone_count, unsigned short pitch)
{
    long vab_vol;
    unsigned long seq_left;
    unsigned long seq_right;
    unsigned long left;
    unsigned long right;
    unsigned short sreg_offset;
    short seq_sep_no;
    SndSeqScore *score;
    unsigned long pan;
    int mask_lo;
    int mask_hi;

    sreg_offset = _svm_cur.voice * SPU_VREG_COUNT;
    vab_vol = _svm_vh->mvol * 0x3fff;
    seq_left = (_svm_cur.vol * vab_vol) / (127 * 127);
    seq_right = (seq_left * _svm_cur.prog_mvol * _svm_cur.tone_vol) / (127 * 127);

    seq_sep_no = _svm_cur.seq_sep_no;
    /* For a utility voice (33) the lookup reads past _ss_score's 32 entries;
     * the score is used only for sequenced voices. */
    score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];
    seq_left = seq_right;
    if (seq_sep_no != SVM_SEQ_UTILITY) {
        seq_left = (seq_right * score->voll) / 127;
        seq_right = (seq_right * score->volr) / 127;
    }

    pan = _svm_cur.tone_pan;
    if (pan < 64) {
        left = seq_left;
        right = (seq_right * pan) / 63;
    } else {
        left = (seq_left * (127 - pan)) / 63;
        right = seq_right;
    }

    pan = _svm_cur.prog_mpan;
    if (pan < 64) {
        right = (right * pan) / 63;
    } else {
        left = (left * (127 - pan)) / 63;
    }

    pan = _svm_cur.pan;
    if (pan < 64) {
        right = (right * pan) / 63;
    } else {
        left = (left * (127 - pan)) / 63;
    }

    if (_svm_stereo_mono == SVM_MODE_MONO) {
        if (left < right) {
            left = right;
        } else {
            right = left;
        }
    }

    /* Direct utility key-ons take linear SPU levels. */
    if (_svm_cur.seq_sep_no != SVM_SEQ_UTILITY) {
        left = (left * left) / 0x3fff;
        right = (right * right) / 0x3fff;
    }

    _svm_sreg_buf[sreg_offset + SPU_VREG_PITCH] = pitch;
    _svm_sreg_buf[sreg_offset + SPU_VREG_VOL_L] = left;
    _svm_sreg_buf[sreg_offset + SPU_VREG_VOL_R] = right;

    _svm_sreg_dirty[_svm_cur.voice] |= SREG_DIRTY_VOL | SREG_DIRTY_PITCH;
    _svm_voice[_svm_cur.voice].pitch = pitch;

    if (_svm_cur.voice < 16) {
        mask_lo = 1 << _svm_cur.voice;
        mask_hi = 0;
    } else {
        mask_lo = 0;
        mask_hi = 1 << (_svm_cur.voice - 16);
    }

    if (_svm_cur.tone_mode & TONE_MODE_REVERB) {
        _svm_orev1 |= mask_lo;
        _svm_orev2 |= mask_hi;
    } else {
        _svm_orev1 &= ~mask_lo;
        _svm_orev2 &= ~mask_hi;
    }

    _svm_onos1 &= ~mask_lo;
    _svm_onos2 &= ~mask_hi;
    _svm_okon1 |= mask_lo;
    _svm_okon2 |= mask_hi;
    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;
}
