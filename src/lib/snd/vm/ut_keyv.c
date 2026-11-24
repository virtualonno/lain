/* Key a chosen SPU voice on with a VAB tone, or off, apart from any sequence. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_aloc2.h"
#include "vm_f.h"
#include "vm_n2p.h"
#include "vm_no1.h"
#include "vm_nowof.h"
#include "vm_nowon.h"
#include "vm_vsu.h"

short SsUtKeyOnV(short voice, short vabId, short prog, short tone, short note,
                 short fine, short voll, short volr)
{
    short tone_index;
    signed char prog_index, voice_tone;
    short vag_id;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice < 0 || voice >= SPU_VOICE_COUNT || _SsVmVSetUp(vabId, prog) != 0) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.seq_sep_no = SVM_SEQ_UTILITY;
    _svm_cur.note = note;
    _svm_cur.fine = fine;
    _svm_cur.tone = tone;
    if (voll == volr) {
        _svm_cur.pan = 64;
        _svm_cur.vol = voll;
    } else if (voll > volr) {
        _svm_cur.vol = voll;
        _svm_cur.pan = (volr * 64) / voll;
    } else {
        _svm_cur.vol = volr;
        _svm_cur.pan = 127 - (voll * 64) / volr;
    }
    _svm_cur.prog_mvol = _svm_pg[prog].mvol;
    _svm_cur.prog_mpan = _svm_pg[prog].mpan;
    _svm_cur.tones = _svm_pg[prog].tones;

    tone_index = _svm_cur.tone + _svm_cur.prog_index * VAB_TONES_PER_PROG;
    _svm_cur.tone_prior = _svm_tn[tone_index].prior;
    vag_id = _svm_tn[tone_index].vag;
    _svm_cur.vag_idx = vag_id;
    _svm_cur.tone_vol = _svm_tn[tone_index].vol;
    _svm_cur.tone_pan = _svm_tn[tone_index].pan;
    _svm_cur.tone_center = _svm_tn[tone_index].center;
    _svm_cur.tone_shift = _svm_tn[tone_index].shift;
    _svm_cur.tone_mode = _svm_tn[tone_index].mode;
    if (vag_id == 0) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.voice = voice;
    _svm_voice[voice].seq_sep_no = SVM_SEQ_UTILITY;
    _svm_voice[voice].vab_id = vabId;
    prog_index = _svm_cur.prog_index;
    _svm_voice[voice].prog = prog;
    _svm_voice[voice].prog_index = prog_index;
    _svm_voice[voice].vag_id = _svm_cur.vag_idx;
    voice_tone = _svm_cur.tone;
    _svm_voice[voice].note = note;
    _svm_voice[voice].keystat = KEYSTAT_TONE;
    _svm_voice[voice].age = 0;
    _svm_voice[voice].tone = voice_tone;
    _SsVmDoAllocate();
    if (_svm_cur.vag_idx == SVM_VAG_NOISE) {
        vmNoiseOn(voice);
    } else {
        _SsVmKeyOnNow(1, note2pitch2(note, fine));
    }
    _snd_ev_flag = 0;
    return voice;
}

short SsUtKeyOffV(short voice)
{
    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice >= 0 && voice < SPU_VOICE_COUNT) {
        _svm_cur.voice = voice;
        _SsVmKeyOffNow(0);
        _snd_ev_flag = 0;
        return 0;
    }
    _snd_ev_flag = 0;
    return -1;
}
