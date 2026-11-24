/* Per-tick sequence volume crescendo step. */

#include <sys/types.h>

#include "snd_int.h"
#include "vm/vm_seq.h"

void _SsSndCrescendo(short sep_no, short seq_no)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned short voll, volr;
    int step;
    int left, right;

    if ((long)score->vol_time < (long)++score->vol_cnt) {
        _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_CRES;
    } else {
        step = (score->vol_delta * (long)score->vol_cnt) / (long)score->vol_time;
        step -= score->vol_remain;
        if (step != 0) {
            score->vol_remain += step;
            _SsVmGetSeqVol(sep_no | ((unsigned short)seq_no << 8), &voll, &volr);
            left = voll + step;
            if (left > 127) {
                left = 127;
            }
            if (left < 0) {
                left = 0;
            }
            right = volr + step;
            if (right > 127) {
                right = 127;
            }
            if (right < 0) {
                right = 0;
            }
            _SsVmSetSeqVol(sep_no | ((unsigned short)seq_no << 8), left, right, 1);
            if ((left == 127 && right == 127) || (left == 0 && right == 0)) {
                _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_CRES;
            }
        }
    }
    _SsVmGetSeqVol(sep_no | ((unsigned short)seq_no << 8), &score->saved_voll, &score->saved_volr);
}
