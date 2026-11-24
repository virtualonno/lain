#include <sys/types.h>
#include "snd_int.h"

void _SsSetProgramChange(short sep_no, short seq_no, unsigned char program)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    score->programs[score->channel] = program;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
