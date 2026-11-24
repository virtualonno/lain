/* MIDI CC 6 (data entry) handler: gives a pending loop its count, or sets the selected RPN or NRPN. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsContDataEntry(short sep_no, short seq_no, unsigned char value)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char channel = score->channel;
    ProgAtr prog_atr;
    VagAtr vag_atr;
    int i;
    int below_center;

    SsUtGetProgAtr((signed char)score->vab_id, score->programs[channel], &prog_atr);

    if (score->loop_count_pending == 1 && score->loop_active == 0) {
        score->loop_count = value;
        score->loop_count_pending = 0;
        score->loop_active = 1;
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        return;
    }

    if (score->rpn_count == 2) {
        if (score->rpn_msb == 0) {
            for (i = 0; i < prog_atr.tones; i++) {
                SsUtGetVagAtr((signed char)score->vab_id, score->programs[channel], i, &vag_atr);
                below_center = 0;
                if (value < 64)
                    below_center = 1;
                switch (score->rpn_lsb) {
                case 0:
                    vag_atr.pbmax = value & 0x7f;
                    vag_atr.pbmin = value & 0x7f;
                    break;
                case 1:
                {
                    unsigned short fine;
                    if (value >= 65 && value <= 127) {
                        if (below_center) {
                            fine = ((value * 100) / 8192) << 13;
                        } else {
                            fine = ((value * 100) / 8192) << 13;
                        }
                    } else {
                        fine = 0;
                    }
                    vag_atr.shift += fine;
                    break;
                }
                case 2:
                {
                    unsigned short coarse;
                    if (value >= 64 && value <= 127) {
                        if (below_center) {
                            coarse = value * 6400;
                        } else {
                            coarse = value * 6400;
                        }
                    } else {
                        coarse = 0;
                    }
                    vag_atr.center += coarse;
                    break;
                }
                }
                SsUtSetVagAtr((signed char)score->vab_id, score->programs[channel], i, &vag_atr);
            }
        }
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        score->rpn_count = 0;
        return;
    }

    if (score->nrpn_count == 2) {
        if (score->nrpn_msb == 0x10) {
            SsFCALL.ccentry[score->nrpn_lsb](
                (signed char)score->vab_id,
                score->programs[channel],
                0,
                vag_atr,
                score->nrpn_lsb,
                value);
        } else {
            SsFCALL.ccentry[score->nrpn_lsb](
                (signed char)score->vab_id,
                score->programs[channel],
                score->nrpn_msb,
                vag_atr,
                score->nrpn_lsb,
                value);
        }
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        score->nrpn_count = 0;
        return;
    }

    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
