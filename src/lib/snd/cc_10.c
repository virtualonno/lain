/* MIDI CC 10 (panpot) handler for LIBSND sequence playback. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm_vol.h"

void _SsContPanpot(short sep_no, short seq_no, unsigned char pan)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char channel = score->channel;

    _SsVmSetVol((short)(sep_no | (seq_no << 8)), (signed char)score->vab_id,
                score->programs[channel], (unsigned short)score->vol[channel], pan);
    score->panpot[channel] = pan;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
