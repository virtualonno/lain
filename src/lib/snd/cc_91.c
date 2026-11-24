/* MIDI CC 91 (external effects / reverb depth) handler for SsFCALL.control[CC_EXTERNAL]. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsContExternal(short sep_no, short seq_no, unsigned char depth)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    SsUtSetReverbDepth(depth, depth);
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
