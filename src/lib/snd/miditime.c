#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

long _SsReadDeltaValue(short sep_no, short seq_no)
{
    SndSeqScore *score;
    unsigned char byte;
    long ticks;

    score = &_ss_score[sep_no][seq_no];
    ticks = *score->read_pos++;
    if (ticks == 0) {
        return 0;
    }
    if (ticks & 0x80) {
        ticks &= 0x7f;
        do {
            byte = *score->read_pos++;
            ticks = (ticks << 7) + (byte & 0x7f);
        } while (byte & 0x80);
    }
    /* Playback timing counts tenths of a MIDI tick. */
    score->delta_total += ticks * 10;
    return ticks * 10;
}
