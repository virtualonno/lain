/* Sequence tick dispatcher: flushes voice updates and steps active scores. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "vm_f.h"

void SsSeqCalledTbyT(void)
{
    int sep_no;
    int seq_no;

    if (_snd_ev_flag == 1) {
        return;
    }
    _snd_ev_flag = 1;
    _SsVmFlush();

    for (sep_no = 0; sep_no < _snd_seq_s_max; sep_no++) {
        if ((_snd_openflag & (1 << sep_no)) != 0) {
            for (seq_no = 0; seq_no < _snd_seq_t_max; seq_no++) {
                if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_PLAY) {
                    _SsSndPlay(sep_no, seq_no);
                    if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_CRES) {
                        _SsSndCrescendo(sep_no, seq_no);
                    }
                    if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_DECRES) {
                        _SsSndCrescendo(sep_no, seq_no);
                    }
                    if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_ACCELE) {
                        _SsSndTempo(sep_no, seq_no);
                    }
                    if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_RIT) {
                        _SsSndTempo(sep_no, seq_no);
                    }
                }
                if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_PAUSE) {
                    _SsSndPause(sep_no, seq_no);
                }
                if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_REPLAY) {
                    _SsSndReplay(sep_no, seq_no);
                }
                if (_ss_score[sep_no][seq_no].flags & SND_SEQ_FLAG_STOP) {
                    _SsSndStop(sep_no, seq_no);
                    _ss_score[sep_no][seq_no].flags = 0;
                }
            }
        }
    }
    _snd_ev_flag = 0;
}
