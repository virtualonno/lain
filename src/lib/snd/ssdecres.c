#include <sys/types.h>
#include "snd_int.h"

inline void _SsSndSetDecres(short sep_no, short seq_no, short delta, long time)
{
    _SsSndSetVolData(sep_no, seq_no, -delta, time);
    _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_DECRES;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_CRES;
}

void SsSeqSetDecrescendo(short seq_no, short delta, long time)
{
    _SsSndSetDecres(seq_no, 0, delta, time);
}

void SsSepSetDecrescendo(short sep_no, short seq_no, short delta, long time)
{
    _SsSndSetDecres(sep_no, seq_no, delta, time);
}
