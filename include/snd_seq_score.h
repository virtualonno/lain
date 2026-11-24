#ifndef SND_SEQ_SCORE_H
#define SND_SEQ_SCORE_H

#include <libsnd.h>

/* SndSeqScore.flags */
#define SND_SEQ_FLAG_PLAY       0x001
#define SND_SEQ_FLAG_PAUSE      0x002
#define SND_SEQ_FLAG_STOP       0x004
#define SND_SEQ_FLAG_REPLAY     0x008
#define SND_SEQ_FLAG_CRES       0x010
#define SND_SEQ_FLAG_DECRES     0x020
#define SND_SEQ_FLAG_ACCELE     0x040
#define SND_SEQ_FLAG_RIT        0x080
#define SND_SEQ_FLAG_NEXT       0x100
#define SND_SEQ_FLAG_EOS        0x200
#define SND_SEQ_FLAG_REPEAT_POS 0x400

#define SND_SEQ_NO_NEXT         (-1)

/* One score of a separated sequence, SS_SEQ_TABSIZ bytes.
 * Delta times count tenths of a MIDI tick. */
typedef struct SndSeqScore {
    unsigned char *read_pos;
    unsigned char *start;
    unsigned char *loop_pos;
    unsigned char *repeat_pos;
    unsigned char *end_pos;      /* with SND_SEQ_FLAG_REPEAT_POS, the score ends once the byte at it is read */
    unsigned char playing;       /* set by play, replay and a move to the next score; cleared by
                                    pause, stop and the end; never read */
    unsigned char loop_active;   /* a counted loop is running */
    unsigned char running_status;
    unsigned char channel;
    unsigned char rpn_lsb;
    unsigned char rpn_msb;
    unsigned char nrpn_lsb;
    unsigned char nrpn_msb;
    unsigned char loop_count_pending; /* a loop start awaits its count in the next data entry */
    unsigned char loop_count;    /* repeats left; from 127 up, for ever */
    unsigned char rpn_count;     /* RPN bytes received; a data entry acts on 2 */
    unsigned char nrpn_count;    /* NRPN bytes received; a data entry acts on 2 */
    signed char play_count;      /* times to play, SSPLAY_INFINITY for ever */
    signed char played;
    signed char next_sep_no;     /* SND_SEQ_NO_NEXT when no score follows */
    signed char next_seq_no;     /* with next_sep_no, the score _SsSeqGetEof starts next */
    unsigned char time_signature[2];
    unsigned char vab_id;
    unsigned char panpot[16];
    unsigned char programs[16];
    unsigned char unused;        /* nothing reads or writes it */
    short vol_delta;
    short vol_remain;
    short vol_step;
    short tempo_step;            /* > 0: calls per tempo step of 1; otherwise minus the step per call */
    short resolution;
    short tick_wait;
    short tick_step;
    short init_tick_step;
    unsigned short voll;
    unsigned short volr;
    unsigned short saved_voll;
    unsigned short saved_volr;
    short vol[16];
    short channel_mute;          /* bit per channel: note-ons on a set channel are not keyed */
    long init_delta;
    long delta_total;            /* delta time read since the score (re)started */
    unsigned long init_tempo;
    long delta;
    unsigned long tempo;
    unsigned long flags;
    unsigned long vol_time;
    unsigned long vol_cnt;
    unsigned long tempo_time;
    long tempo_cnt;
    unsigned long tempo_target;
} SndSeqScore;

#endif
