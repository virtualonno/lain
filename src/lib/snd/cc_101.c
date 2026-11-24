/* MIDI CC 101 (RPN MSB) control-change handler for LIBSND sequence playback. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsContRpn2(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    score->rpn_msb = value;
    ++score->rpn_count;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
