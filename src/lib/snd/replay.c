#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsSndReplay(short sep_no, short seq_no)
{
    SndSeqScore *score = _ss_score[sep_no] + seq_no;

    score->playing = 1;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_REPLAY;
}
