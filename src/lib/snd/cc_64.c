/* LIBSND cc_64: MIDI CC 64 (damper pedal) handler for SsFCALL.control[CC_DAMPER]. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

extern void _SsVmDamperOn(void);
extern void _SsVmDamperOff(void);

void _SsContDamper(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    if (value < 64)
        _SsVmDamperOff();
    else
        _SsVmDamperOn();

    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
