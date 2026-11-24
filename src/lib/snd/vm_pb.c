/* Pitch bend: retune every voice playing a sequence's program. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_vm_int.h"
#include "vm_f.h"
#include "vm_pb.h"
#include "vm/vm_n2p.h"
#include "vm/vm_vsu.h"

/* Retune voice vc to its note bent by bend (64 = centre) when it plays seq_sep_no's
   program prog of vab_id. Returns 1 when it did. */
short _SsVmPBVoice(short vc, short seq_sep_no, short vab_id, short prog, unsigned short bend)
{
    short offset = bend - 64;
    unsigned short tone_index;
    unsigned short base;
    unsigned short note;
    unsigned short fine;
    long up;
    long down;
    long semitones;

    if (_svm_voice[vc].seq_sep_no == seq_sep_no) {
        if (_svm_voice[vc].vab_id == vab_id) {
            if (_svm_voice[vc].prog == prog) {
                base = _svm_voice[vc].note;
                tone_index = _svm_voice[vc].tone + _svm_cur.prog_index * VAB_TONES_PER_PROG;
                if (offset > 0) {
                    up = offset * _svm_tn[tone_index].pbmax;
                    note = base + up / 63;
                    fine = (up % 63) * 2;
                } else if (offset < 0) {
                    down = offset * _svm_tn[tone_index].pbmin;
                    semitones = down / 64;
                    note = base + semitones - 1;
                    fine = (down - semitones * 64) * 2 + 127;
                } else {
                    note = base;
                    fine = 0;
                }
                _svm_cur.tone = _svm_voice[vc].tone;
                _svm_cur.voice = vc;
                _svm_sreg_buf[vc * SPU_VREG_COUNT + SPU_VREG_PITCH] = note2pitch2(note, fine);
                _svm_sreg_dirty[vc] |= SREG_DIRTY_PITCH;
                return 1;
            }
        }
    }
    return 0;
}

int _SsVmPitchBend(short seq_sep_no, short vab_id, short prog, unsigned short bend)
{
    short vc;
    int count;

    _SsVmVSetUp(vab_id, prog);
    _svm_cur.seq_sep_no = seq_sep_no;
    count = 0;
    for (vc = 0; vc < _SsVmMaxVoice; vc++)
        count += _SsVmPBVoice(vc, seq_sep_no, vab_id, prog, bend);
    return count;
}
