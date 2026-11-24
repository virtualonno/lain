#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm/vm_seq.h"

void Snd_SetPlayMode(short sep_no, short seq_no, signed char mode, short count)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    score->read_pos = score->start;
    score->loop_pos = score->start;
    score->repeat_pos = score->start;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_EOS;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_STOP;
    score->play_count = count;

    if (mode == SSPLAY_PLAY) {
        _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_PLAY;
        score->playing = 1;
        score->played = 0;
        _SsVmSetSeqVol((short)(sep_no | (seq_no << 8)), score->voll, score->volr, 1);
    } else if (mode == SSPLAY_PAUSE) {
        _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_PAUSE;
    }
}
