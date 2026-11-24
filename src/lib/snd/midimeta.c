#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsGetMetaEvent(short sep_no, short seq_no, unsigned char type)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    int high = *score->read_pos++ << 16;
    int middle = *score->read_pos++ << 8;
    unsigned char *cursor = score->read_pos;
    int low = *cursor;
    unsigned long midi_ticks_per_minute, tick_rate;

    /* The tempo event encodes microseconds per beat. */
    score->tempo = MICROSECONDS_PER_MINUTE / (low | (high | middle));
    midi_ticks_per_minute = score->resolution * score->tempo;
    score->read_pos = cursor + 1;
    tick_rate = VBLANK_MINUS;

    if (midi_ticks_per_minute * 10 < tick_rate * 60) {
        score->tick_wait = tick_rate * 600 / midi_ticks_per_minute;
        score->tick_step = score->tick_wait;
    } else {
        unsigned long step = (score->resolution * score->tempo * 10) / (tick_rate * 60);
        unsigned long fraction;

        score->tick_step = step;
        fraction = (score->resolution * score->tempo * 10) % (tick_rate * 60);
        score->tick_wait = -1;
        if (tick_rate * 30 < fraction) {
            score->tick_step = step + 1;
        }
    }
    score->delta = _SsReadDeltaValue(sep_no, seq_no);
}
