#ifndef SND_SEQ_TICK_ENV_H
#define SND_SEQ_TICK_ENV_H

#include "psx_hw.h"

/* SndSeqTickEnv.tick_irq while no interrupt drives the sequencer. */
#define SND_TICK_IRQ_NONE   0x7f

/* How the sequencer is ticked: SsSetTickMode sets the mode, SsStart arms the counter or VBLANK. */
typedef struct SndSeqTickEnv {
    long tick_mode;                 /* SS_TICK60 .. SS_TICKVSYNC, or ticks per second */
    long manual;                    /* SS_NOTICK: the program calls SsSeqCalledTbyT itself */
    void (*callback)();             /* the sequencer's tick */
    void (*vsync_callback)();       /* the VBLANK handler found when the tick shares VBLANK */
    char vsync_tick;                /* tick through VSyncCallback */
    char every_other;               /* the counter runs twice as fast as the tick */
    char tick_irq;                  /* PSX_IRQ_VBLANK/PSX_IRQ_RCNT2, or SND_TICK_IRQ_NONE when stopped */
} SndSeqTickEnv;

#endif
