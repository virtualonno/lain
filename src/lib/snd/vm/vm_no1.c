/* Key on an SPU voice as a noise generator using current voice-manager state. */

#include <sys/types.h>
#include <libsnd.h>
#include <libspu.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_f.h"
#include "vm_no1.h"

extern long _snd_vmask;

void vmNoiseOn(unsigned char voice)
{
    long vab_vol;
    unsigned long seq_left;
    unsigned long seq_right;
    unsigned long left;
    unsigned long right;
    short seq_sep_no;
    SndSeqScore *score;
    unsigned long pan;
    int voice_index;
    short i;
    int mask_lo;
    int mask_hi;

    seq_sep_no = _svm_cur.seq_sep_no;
    vab_vol = _svm_vh->mvol * 0x3fff;
    seq_left = (_svm_cur.vol * vab_vol) / (127 * 127);
    seq_right = (seq_left * _svm_cur.prog_mvol * _svm_cur.tone_vol) / (127 * 127);

    /* For utility notes the lookup reads beyond the pointer table, but
     * neither track volume is fetched. */
    score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];
    seq_left = seq_right;
    if (seq_sep_no != SVM_SEQ_UTILITY) {
        seq_left = (seq_right * score->voll) / 127;
        seq_right = (seq_right * score->volr) / 127;
    }

    pan = (signed char)_svm_cur.tone_pan;
    if (pan < 64) {
        left = seq_left;
        right = (seq_right * pan) / 63;
    } else {
        left = (seq_left * (127 - pan)) / 63;
        right = seq_right;
    }

    pan = (signed char)_svm_cur.prog_mpan;
    if (pan < 64) {
        right = (right * pan) / 63;
    } else {
        left = (left * (127 - pan)) / 63;
    }

    pan = (signed char)_svm_cur.pan;
    if (pan < 64) {
        right = (pan * right) / 63;
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

    voice_index = voice;
    SpuSetNoiseClock((_svm_cur.note - (signed char)_svm_cur.tone_center) &
                     SPUCNT_NOISE_CLK_MASK);
    _svm_sreg_buf[voice_index * SPU_VREG_COUNT + SPU_VREG_VOL_R] = right;
    _svm_sreg_buf[voice_index * SPU_VREG_COUNT + SPU_VREG_VOL_L] = left;
    _svm_sreg_dirty[voice_index] |= SREG_DIRTY_VOL;

    if ((unsigned int)voice_index < 16) {
        mask_lo = 1 << voice_index;
        mask_hi = 0;
    } else {
        mask_lo = 0;
        mask_hi = 1 << (voice_index - 16);
    }

    _svm_voice[voice].pitch = 10;
    for (i = 0; i < _SsVmMaxVoice; i++) {
        if (!(_snd_vmask & (1 << i))) {
            _svm_voice[i].keystat &= KEYSTAT_TONE;
        }
    }
    _svm_voice[voice].keystat = KEYSTAT_NOISE;

    _svm_okon1 |= mask_lo;
    _svm_okon2 |= mask_hi;
    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;

    if (_svm_cur.tone_mode & TONE_MODE_REVERB) {
        _svm_orev1 |= mask_lo;
        _svm_orev2 |= mask_hi;
    } else {
        _svm_orev1 &= ~mask_lo;
        _svm_orev2 &= ~mask_hi;
    }

    _svm_onos1 = mask_lo;
    _svm_onos2 = mask_hi;
}
