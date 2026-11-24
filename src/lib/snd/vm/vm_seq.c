/* Set a sequence's volume into the buffered SPU volume of each voice it plays, read it back, key its voices off. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_f.h"
#include "vm_nowof.h"
#include "vm_seq.h"

extern long _snd_vmask;
#include "vm_vsu.h"

short _SsVmSetSeqVol(short seq_sep_no, unsigned short voll, unsigned short volr, short unused)
{
    SndSeqScore *score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];
    short voice;

    score->voll = voll;
    score->volr = volr;
    if (score->voll >= 127) {
        score->voll = 127;
    }
    if (score->volr >= 127) {
        score->volr = 127;
    }

    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        long level;
        unsigned long left, right;
        unsigned char pan;

        if (_snd_vmask & (1 << voice)) {
            continue;
        }
        if (_svm_voice[voice].seq_sep_no != seq_sep_no) {
            continue;
        }
        if (_svm_voice[voice].vab_id != (signed char)score->vab_id) {
            continue;
        }

        _SsVmVSetUp(_svm_voice[voice].vab_id, _svm_voice[voice].prog_index);
        level = ((_svm_voice[voice].vol * score->vol[_svm_voice[voice].channel]) / 127) * 0x3fff;
        left = (_svm_vh->mvol * level) / (127 * 127);
        right = (left * _svm_pg[_svm_voice[voice].prog].mvol *
                 _svm_tn[_svm_voice[voice].prog_index * VAB_TONES_PER_PROG +
                         _svm_voice[voice].tone].vol) / (127 * 127);
        left = (right * score->voll) / 127;
        pan = _svm_tn[_svm_voice[voice].prog_index * VAB_TONES_PER_PROG + _svm_voice[voice].tone].pan;
        right = (right * score->volr) / 127;
        if (pan < 64) {
            voll = left;
            volr = (right * pan) / 63;
        } else {
            volr = right;
            voll = (left * (127 - pan)) / 63;
        }

        pan = _svm_pg[_svm_voice[voice].prog].mpan;
        if (pan < 64) {
            volr = (volr * pan) / 63;
        } else {
            voll = (voll * (127 - pan)) / 63;
        }

        pan = _svm_voice[voice].pan;
        if (pan < 64) {
            volr = (volr * pan) / 63;
        } else {
            voll = (voll * (127 - pan)) / 63;
        }

        if (_svm_stereo_mono == SVM_MODE_MONO) {
            if (voll < volr) {
                voll = volr;
            } else {
                volr = voll;
            }
        }

        voll = (voll * voll) / 0x3fff;
        _svm_sreg_buf[voice * SPU_VREG_COUNT + SPU_VREG_VOL_L] = voll;
        volr = (volr * volr) / 0x3fff;
        _svm_sreg_buf[voice * SPU_VREG_COUNT + SPU_VREG_VOL_R] = volr;
        _svm_sreg_dirty[voice] |= SREG_DIRTY_VOL;
    }
    return seq_sep_no;
}

short _SsVmGetSeqVol(short seq_sep_no, unsigned short *voll, unsigned short *volr)
{
    SndSeqScore *score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];

    _svm_cur.seq_sep_no = seq_sep_no;
    *voll = score->voll;
    *volr = score->volr;
    return _svm_cur.seq_sep_no;
}

short _SsVmGetSeqLVol(short seq_sep_no)
{
    SndSeqScore *score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];

    _svm_cur.seq_sep_no = seq_sep_no;
    return score->voll;
}

short _SsVmGetSeqRVol(short seq_sep_no)
{
    SndSeqScore *score = &_ss_score[seq_sep_no & 0xff][(seq_sep_no & 0xff00) >> 8];

    _svm_cur.seq_sep_no = seq_sep_no;
    return score->volr;
}

void _SsVmSeqKeyOff(short seq_sep_no)
{
    unsigned char voice;

    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        if (!(_snd_vmask & (1 << voice)) && _svm_voice[voice].seq_sep_no == seq_sep_no) {
            _svm_cur.voice = voice;
            _SsVmKeyOffNow(0);
        }
    }
}
