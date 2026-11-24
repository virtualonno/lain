#ifndef SND_INT_H
#define SND_INT_H

#include <libsnd.h>

#include "snd_seq_score.h"
#include "snd_seq_tick_env.h"

/* Maximum open SEQ/SEP slots and tracks per SEP in LIBSND. */
#define SND_SEQ_SEP_MAX         32
#define SND_SEQ_TRACK_MAX       16

#define MICROSECONDS_PER_MINUTE 60000000L

extern SsMarkCallbackProc _SsMarkCallback[SND_SEQ_SEP_MAX][SND_SEQ_TRACK_MAX];
extern SndSeqScore *_ss_score[SND_SEQ_SEP_MAX];
extern long VBLANK_MINUS;
extern unsigned long _snd_openflag;
extern short _snd_seq_s_max;
extern short _snd_seq_t_max;
extern long _snd_ev_flag;

extern void _SsInit(void);
extern int _SsInitSoundSeq(short sep_no, short vab_id, unsigned char *data);
extern void _SsSndPlay(short sep_no, short seq_no);
extern void _SsSeqPlay(short sep_no, short seq_no);
extern void _SsSeqGetEof(short sep_no, short seq_no, unsigned char ev);
extern int _SsGetSeqData(short sep_no, short seq_no);
extern long _SsReadDeltaValue(short sep_no, short seq_no);
extern void _SsSndNextSep(short sep_no, short seq_no);
extern void _SsSndReplay(short sep_no, short seq_no);
extern void _SsSndStop(short sep_no, short seq_no);
extern void _SsSndCrescendo(short sep_no, short seq_no);
extern void _SsSndSetVolData(short sep_no, short seq_no, short delta, long time);
extern void _SsSndPause(short sep_no, short seq_no);
extern void _SsSndTempo(short sep_no, short seq_no);
extern void Snd_SetPlayMode(short sep_no, short seq_no, signed char mode, short count);

extern SndSeqTickEnv _snd_seq_tick_env;

#endif
