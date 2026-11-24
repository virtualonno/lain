/* MIDI CC 98 (NRPN LSB) handler for LIBSND sequence playback. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

#include "snd_nrpn.h"

void _SsContNrpn1(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    if (score->nrpn_msb == NRPN_MARK) {
        SsMarkCallbackProc callback = _SsMarkCallback[sep_no][seq_no];

        if (callback)
            callback(sep_no, seq_no, value);
    }

    if (score->nrpn_msb != NRPN_LOOP_END &&
        score->nrpn_msb != NRPN_LOOP_START &&
        score->nrpn_msb != NRPN_MARK) {
        score->nrpn_lsb = value;
        score->loop_count_pending = 0;
        ++score->nrpn_count;
    }
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
