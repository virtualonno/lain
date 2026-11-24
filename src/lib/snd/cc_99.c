/* MIDI CC 99 (NRPN MSB) handler: a score's loop start and loop end. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

#include "snd_nrpn.h"

enum { LOOP_COUNT_INFINITE = 127 };

void _SsContNrpn2(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];

    switch (value) {
    case NRPN_LOOP_START:
        score->nrpn_msb = value;
        score->loop_count_pending = 1;
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        score->loop_pos = score->read_pos;
        return;
    case NRPN_LOOP_END:
        score->nrpn_msb = value;
        if (score->loop_count == 0) {
            score->loop_active = 0;
            score->delta = _SsReadDeltaValue(sep_no, seq_no);
            return;
        }
        if (score->loop_count < LOOP_COUNT_INFINITE) {
            --score->loop_count;
            score->delta = _SsReadDeltaValue(sep_no, seq_no);
            if (score->loop_count != 0) {
                score->read_pos = score->loop_pos;
            } else {
                score->loop_active = 0;
            }
        } else {
            _SsReadDeltaValue(sep_no, seq_no);
            score->delta = 0;
            score->read_pos = score->loop_pos;
        }
        return;
    default:
        score->nrpn_msb = value;
        ++score->nrpn_count;
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        return;
    }
}
