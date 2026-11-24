/* LIBSND MIDI CC 11 (Expression) handler: updates channel program and voice volume. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm_prog.h"
#include "vm_vol.h"

void _SsContExpression(short sep_no, short seq_no, unsigned char expression)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char channel = score->channel;

    _SsVmSetProgVol((signed char)score->vab_id, score->programs[channel], expression);
    _SsVmSetVol((short)(sep_no | (seq_no << 8)), (signed char)score->vab_id,
                score->programs[channel], score->vol[channel], score->panpot[channel]);
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
