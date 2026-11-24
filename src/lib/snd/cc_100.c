/* MIDI CC 100 (RPN LSB) handler for LIBSND sequence playback. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsContRpn1(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    score->rpn_lsb = value;
    ++score->rpn_count;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
