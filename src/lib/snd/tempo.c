/* Per-tick sequence tempo ramp (accelerando / ritardando) handler. */

#include "snd_int.h"

void _SsSndTempo(short sep_no, short seq_no)
{
    SndSeqScore *score;

    score = &_ss_score[sep_no][seq_no];
    if (--score->tempo_cnt < 0) {
        _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_ACCELE;
        _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_RIT;
        return;
    }

    if (score->tempo_step > 0) {
        if (score->tempo_cnt % score->tempo_step == 0) {
            if (score->tempo > score->tempo_target) {
                score->tempo--;
            } else if (score->tempo < score->tempo_target) {
                score->tempo++;
            }
            score->tick_step = (score->resolution * score->tempo * 10) / (VBLANK_MINUS * 60);
            if (score->tick_step <= 0) {
                score->tick_step = 1;
            }
            if (score->tempo_cnt == 0 || score->tempo == score->tempo_target) {
                _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_ACCELE;
                _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_RIT;
            }
        }
    } else {
        if (score->tempo > score->tempo_target) {
            score->tempo += score->tempo_step;
            if (score->tempo < score->tempo_target) {
                score->tempo = score->tempo_target;
            }
        } else if (score->tempo < score->tempo_target) {
            score->tempo -= score->tempo_step;
            if (score->tempo > score->tempo_target) {
                score->tempo = score->tempo_target;
            }
        }
        score->tick_step = (score->resolution * score->tempo * 10) / (VBLANK_MINUS * 60);
        if (score->tick_step <= 0) {
            score->tick_step = 1;
        }
        if (score->tempo_cnt == 0 || score->tempo == score->tempo_target) {
            _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_ACCELE;
            _ss_score[sep_no][seq_no].flags &= ~SND_SEQ_FLAG_RIT;
        }
    }
}
