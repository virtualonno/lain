#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_f.h"
#include "vm_vol.h"
#include "vm/vm_vsu.h"

extern long _snd_vmask;

int _SsVmSetVol(int seq_sep_no, int vab_id, int prog, unsigned short volume, unsigned short pan)
{
    int count = 0;
    short voice;
    SndSeqScore *score = &_ss_score[seq_sep_no & 0xff][((short)seq_sep_no & 0xff00) >> 8];
    unsigned long lvol, rvol;
    int master_gain, initial_gain;
    unsigned long left, right;
    unsigned char balance;
    int tone_index;

    _SsVmVSetUp((short)vab_id, (short)prog);
    _svm_cur.seq_sep_no = seq_sep_no;
    /* kept: the plain form compiles differently; cause not established */
    do {
        if (pan == 0) {
            pan = 1;
        }
    } while (0);
    if (volume == 0) {
        volume = 1;
    }

    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        if (_snd_vmask & (1 << voice)) {
            continue;
        }
        if (_svm_voice[voice].seq_sep_no != (short)seq_sep_no ||
            _svm_voice[voice].prog != (short)prog ||
            _svm_voice[voice].vab_id != (short)vab_id) {
            continue;
        }

        if (score->vol[score->channel] != volume) {
            if (score->vol[score->channel] == 0) {
                score->vol[score->channel] = 1;
            }
            /* kept: overwritten below; without it the compiled code differs */
            initial_gain = _svm_voice[voice].vol * volume / 127;
            master_gain = _svm_vh->mvol * 0x3fff;
            lvol = initial_gain * master_gain / (127 * 127);
        }
        initial_gain = _svm_voice[voice].vol * volume / 127;
        master_gain = _svm_vh->mvol * 0x3fff;
        lvol = initial_gain * master_gain / (127 * 127);
        tone_index = _svm_voice[voice].prog_index * VAB_TONES_PER_PROG + _svm_voice[voice].tone;
        rvol = (lvol * _svm_pg[(short)prog].mvol * _svm_tn[tone_index].vol) / (127 * 127);
        lvol = (rvol * score->voll) / 127;
        rvol = (rvol * score->volr) / 127;

        balance = _svm_tn[tone_index].pan;
        if (balance < 64) {
            left = lvol;
            right = (rvol * balance) / 63;
        } else {
            right = rvol;
            left = (lvol * (127 - balance)) / 63;
        }
        /* kept: the plain form compiles differently; cause not established */
        do {
            do {
                balance = _svm_pg[_svm_voice[voice].prog].mpan;
            } while (0);
        } while (0);
        if (balance < 64) {
            right = (right * balance) / 63;
        } else {
            left = (left * (127 - balance)) / 63;
        }
        balance = (unsigned char)pan;
        if (balance < 64) {
            right = (right * balance) / 63;
        } else {
            left = (left * (127 - balance)) / 63;
        }

        if (_svm_stereo_mono == SVM_MODE_MONO) {
            if (left < right) {
                left = right;
            } else {
                right = left;
            }
        }
        left = (left * left) / 0x3fff;
        right = (right * right) / 0x3fff;
        _svm_sreg_buf[voice * SPU_VREG_COUNT + SPU_VREG_VOL_L] = left;
        _svm_sreg_buf[voice * SPU_VREG_COUNT + SPU_VREG_VOL_R] = right;
        _svm_sreg_dirty[voice] |= SREG_DIRTY_VOL;
        count++;
    }
    return count;
}
