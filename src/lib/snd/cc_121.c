/* MIDI CC 121 (Reset All Controllers) handler for SsFCALL.control[CC_RESETALL]. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

extern void _SsVmDamperOff(void);

enum { MIDI_CHANNEL_VOLUME_MAX = 127, MIDI_PAN_CENTER = 64 };

void _SsContResetAll(short sep_no, short seq_no)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    SsUtReverbOff();
    _SsVmDamperOff();
    score->programs[score->channel] = score->channel;
    score->rpn_lsb = 0;
    score->rpn_msb = 0;
    score->vol[score->channel] = MIDI_CHANNEL_VOLUME_MAX;
    score->panpot[score->channel] = MIDI_PAN_CENTER;
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
