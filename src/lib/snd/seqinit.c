#include <sys/types.h>
#include <stdio.h>

#include "snd_int.h"

int _SsInitSoundSeq(short sep_no, short vab_id, unsigned char *data)
{
    SndSeqScore *score;
    int channel;
    unsigned char first_byte;
    int high, middle, low;
    long us_per_beat, bpm, remainder;
    long delta;
    unsigned long midi_ticks_per_minute;
    unsigned long step, fraction;

    channel = 0;
    score = _ss_score[sep_no];
    score->vab_id = vab_id;
    score->resolution = 0;
    score->rpn_lsb = 0;
    score->rpn_msb = 0;
    score->rpn_count = 0;
    score->nrpn_lsb = 0;
    score->nrpn_msb = 0;
    score->nrpn_count = 0;
    score->channel = 0;
    score->init_delta = 0;
    score->delta_total = 0;
    score->init_tempo = 0;
    score->init_tick_step = 0;
    score->played = 0;
    score->play_count = 1;
    score->playing = 0;
    score->delta = 0;
    score->loop_count_pending = 0;
    score->loop_count = 0;
    score->loop_active = 0;
    score->running_status = 0;
    score->channel_mute = 0;
    score->time_signature[0] = 0;
    score->time_signature[1] = 0;
    do {
        score->programs[channel] = channel;
        score->panpot[channel] = 64;
        score->vol[channel] = 127;
    } while (++channel < 16);
    score->tick_wait = 1;
    score->read_pos = data;
    first_byte = *data;
    if (first_byte == 'S' || first_byte == 'p') {
        score->read_pos += 7;
        if (*score->read_pos++ != 1) {
            printf("This is not SEQ Data.\n");
            return -1;
        }
    } else {
        printf("This is an old SEQ Data Format.\n");
        return 0;
    }
    {
        int msb = *score->read_pos++;
        int lsb = *score->read_pos++;

        score->resolution = lsb | msb << 8;
    }
    high = *score->read_pos++ << 16;
    middle = *score->read_pos++ << 8;
    low = *score->read_pos;
    us_per_beat = low | (high | middle);
    bpm = MICROSECONDS_PER_MINUTE / us_per_beat;
    remainder = MICROSECONDS_PER_MINUTE % us_per_beat;
    score->read_pos++;
    score->init_tempo = us_per_beat;
    if ((long)(score->init_tempo >> 1) < remainder) {
        score->init_tempo = bpm + 1;
    } else {
        score->init_tempo = bpm;
    }
    score->tempo = score->init_tempo;
    score->time_signature[0] = *score->read_pos++;
    score->time_signature[1] = *score->read_pos++;
    delta = _SsReadDeltaValue(sep_no, 0);
    midi_ticks_per_minute = score->resolution * score->init_tempo;
    score->start = score->read_pos;
    score->loop_pos = score->read_pos;
    score->repeat_pos = score->read_pos;
    score->init_delta = delta;
    score->delta = delta;
    score->end_pos = 0;
    if (midi_ticks_per_minute * 10 < VBLANK_MINUS * 60) {
        score->tick_wait = VBLANK_MINUS * 600 / midi_ticks_per_minute;
        score->tick_step = score->tick_wait;
    } else {
        step = score->resolution * score->init_tempo * 10 / (VBLANK_MINUS * 60);
        score->tick_step = step;
        fraction = score->resolution * score->init_tempo * 10 % (VBLANK_MINUS * 60);
        score->tick_wait = -1;
        if ((unsigned long)(VBLANK_MINUS * 30) < fraction) {
            score->tick_step = step + 1;
        }
    }
    score->init_tick_step = score->tick_step;
    return 0;
}
