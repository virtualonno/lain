#include <sys/types.h>
#include "snd_int.h"

inline void _SsSndSetCres(short sep_no, short seq_no, short delta, long time)
{
    _SsSndSetVolData(sep_no, seq_no, delta, time);
    _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_CRES;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_DECRES;
}

void SsSeqSetCrescendo(short seq_no, short delta, long time)
{
    _SsSndSetCres(seq_no, 0, delta, time);
}

void SsSepSetCrescendo(short sep_no, short seq_no, short delta, long time)
{
    _SsSndSetCres(sep_no, seq_no, delta, time);
}
