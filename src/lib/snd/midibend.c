#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm_pb.h"

void _SsSetPitchBend(short sep_no, short seq_no)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char bend = *score->read_pos++;

    _SsVmPitchBend(sep_no | (seq_no << 8), (signed char)score->vab_id,
                   *(score->channel + score->programs), bend);
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
