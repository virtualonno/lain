#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm/vm_seq.h"

extern void _SsVmDamperOff(void);

void _SsSndStop(short sep_no, short seq_no)
{
    SndSeqScore *score;
    int i;

    score = &_ss_score[sep_no][seq_no];
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_PLAY;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_PAUSE;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_REPLAY;
    _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_REPEAT_POS;
    _ss_score[sep_no][seq_no].flags |= SND_SEQ_FLAG_STOP;
    _SsVmSeqKeyOff(sep_no | ((unsigned short)seq_no << 8));
    _SsVmDamperOff();

    score->playing = 0;
    score->delta_total = 0;
    score->loop_count_pending = 0;
    score->rpn_lsb = 0;
    score->rpn_msb = 0;
    score->rpn_count = 0;
    score->nrpn_lsb = 0;
    score->nrpn_msb = 0;
    score->nrpn_count = 0;
    score->channel = 0;
    score->played = 0;
    score->loop_count_pending = 0; /* the original clears it twice */
    score->loop_count = 0;
    score->loop_active = 0;
    score->running_status = 0;

    score->delta = score->init_delta;
    score->tempo = score->init_tempo;
    score->tick_step = score->init_tick_step;
    score->read_pos = score->start;
    score->loop_pos = score->start;

    for (i = 0; i < 16; i++) {
        score->programs[i] = i;
        score->panpot[i] = 0x40;
        score->vol[i] = 0x7f;
    }

    score->saved_voll = 0x7f;
    score->saved_volr = 0x7f;
}

void SsSeqStop(short seq_access_num)
{
    _SsSndStop(seq_access_num, 0);
}

void SsSepStop(short sep_access_num, short seq_num)
{
    _SsSndStop(sep_access_num, seq_num);
}
