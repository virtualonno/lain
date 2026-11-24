#include <sys/types.h>
#include "snd_int.h"

void _SsSndSetVolData(short sep_no, short seq_no, short delta, long time)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned long flags = score->flags;

    if (flags & SND_SEQ_FLAG_STOP)
        return;
    if (flags & SND_SEQ_FLAG_NEXT)
        return;
    if (delta == 0)
        return;

    score->vol_delta = delta;
    score->vol_time = time;
    score->vol_cnt = 0;
    score->vol_remain = 0;
}
