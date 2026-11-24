#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm/vm_seq.h"

inline void _SsSndSetVol(short sep_no, short seq_no, short left, short right)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    if (score->flags != SND_SEQ_FLAG_PLAY) {
        score->voll = left;
        score->volr = right;
    } else {
        _SsVmSetSeqVol((short)(sep_no | (seq_no << 8)),
                       (unsigned short)left, (unsigned short)right, 1);
    }
}

void SsSeqSetVol(short sep_no, short left, short right)
{
    _SsSndSetVol(sep_no, 0, left, right);
}

void SsSepSetVol(short sep_no, short seq_no, short left, short right)
{
    _SsSndSetVol(sep_no, seq_no, left, right);
}

void SsSeqGetVol(short sep_no, short seq_no, short *left, short *right)
{
    _SsVmGetSeqVol((short)(sep_no | (seq_no << 8)),
                   (unsigned short *)left, (unsigned short *)right);
}
