/* Assign and initialize the separated sequence score table. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void SsSetTableSize(char *table, short s_max, short t_max)
{
    int sep_no;
    int seq_no;

    _snd_seq_s_max = s_max;
    _snd_seq_t_max = t_max;

    for (sep_no = 0; sep_no < s_max; sep_no++) {
        _ss_score[sep_no] = (SndSeqScore *)(table + sep_no * t_max * SS_SEQ_TABSIZ);
    }

    /* Reserve the slots beyond the caller's table. */
    for (sep_no = s_max; sep_no < SND_SEQ_SEP_MAX; sep_no++) {
        _snd_openflag |= 1 << sep_no;
    }

    for (sep_no = 0; sep_no < _snd_seq_s_max; sep_no++) {
        for (seq_no = 0; seq_no < _snd_seq_t_max; seq_no++) {
            _ss_score[sep_no][seq_no].flags = 0;
            _ss_score[sep_no][seq_no].next_sep_no = SND_SEQ_NO_NEXT;
            _ss_score[sep_no][seq_no].next_seq_no = 0;
            _ss_score[sep_no][seq_no].vol_delta = 0;
            _ss_score[sep_no][seq_no].vol_remain = 0;
            _ss_score[sep_no][seq_no].vol_time = 0;
            _ss_score[sep_no][seq_no].vol_cnt = 0;
            _ss_score[sep_no][seq_no].vol_step = 0;
            _ss_score[sep_no][seq_no].tempo_target = 0;
            _ss_score[sep_no][seq_no].tempo_cnt = 0;
            _ss_score[sep_no][seq_no].tempo_time = 0;
            _ss_score[sep_no][seq_no].tempo_step = 0;
            _ss_score[sep_no][seq_no].voll = 0x7f;
            _ss_score[sep_no][seq_no].volr = 0x7f;
            _ss_score[sep_no][seq_no].saved_voll = 0x7f;
            _ss_score[sep_no][seq_no].saved_volr = 0x7f;
        }
    }
}
