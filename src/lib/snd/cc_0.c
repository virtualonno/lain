#include <sys/types.h>
#include "snd_int.h"

void _SsContBankChange(short sep_no, short seq_no)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    score->vab_id = *score->read_pos++;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
