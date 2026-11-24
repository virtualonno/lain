/* Start the queued follow-up score of a separated sequence. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsSndNextSep(short sep_no, short seq_no)
{
    SndSeqScore *score;

    score = _ss_score[sep_no] + seq_no;
    score->play_count = 1;
    score->played = 0;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_NEXT;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_REPLAY;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_PAUSE;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_STOP;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_EOS;
    score->playing = 1;
    score->read_pos = score->start;
    _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_PLAY;
}
