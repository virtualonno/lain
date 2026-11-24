/* Arm the sequencer tick on root counter 2 or on VBLANK. */

#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include <libsnd.h>

#include "etc_int.h"
#include "snd_int.h"
#include "snd_seq_tick_env.h"

static void _SsTrapIntrVSync(void);
static void _SsSeqCalledTbyTEveryOther(void);

SndSeqTickEnv _snd_seq_tick_env = { 60, 1, SsSeqCalledTbyT, NULL, 0, 0, SND_TICK_IRQ_NONE };

static void _SsStart(long direct_vsync_irq)
{
    long i;
    unsigned long counter;
    unsigned short target;

    for (i = 0; i < 1000; i++) {
    }
    counter = RCntCNT2;
    _snd_seq_tick_env.vsync_tick = 0;
    _snd_seq_tick_env.tick_irq = PSX_IRQ_RCNT2;
    _snd_seq_tick_env.every_other = 0;
    _snd_seq_tick_env.vsync_callback = NULL;
    /* SetRCnt runs root counter 2 at the system clock / 8, 4,233,600 Hz. */
    target = 17640;
    switch (_snd_seq_tick_env.tick_mode) {
    case SS_NOTICK0:
        _snd_seq_tick_env.tick_irq = SND_TICK_IRQ_NONE;
        return;
    case SS_TICK240:
        break;
    case SS_TICKVSYNC:
        _snd_seq_tick_env.tick_irq = PSX_IRQ_VBLANK;
        if (direct_vsync_irq == 0) {
            _snd_seq_tick_env.vsync_tick = 1;
        } else {
            counter = RCntCNT3;
            target = 1;
        }
        break;
    case SS_TICK120:
        target = 35280;
        break;
    default:
        if (_snd_seq_tick_env.manual) {
            return;
        }
        if (_snd_seq_tick_env.tick_mode < 70) {
            target = 2116800 / _snd_seq_tick_env.tick_mode;
            _snd_seq_tick_env.every_other++;
        } else {
            target = 4233600 / _snd_seq_tick_env.tick_mode;
        }
        break;
    }
    if (_snd_seq_tick_env.vsync_tick) {
        EnterCriticalSection();
        VSyncCallback(_snd_seq_tick_env.callback);
    } else {
        EnterCriticalSection();
        ResetRCnt(counter);
        SetRCnt(counter, target, RCntMdINTR);
        if (_snd_seq_tick_env.tick_irq == PSX_IRQ_VBLANK) {
            _snd_seq_tick_env.vsync_callback = InterruptCallback(PSX_IRQ_VBLANK, NULL);
            InterruptCallback(_snd_seq_tick_env.tick_irq, _SsTrapIntrVSync);
        } else if (_snd_seq_tick_env.every_other == 0) {
            InterruptCallback(_snd_seq_tick_env.tick_irq, _snd_seq_tick_env.callback);
        } else {
            InterruptCallback(_snd_seq_tick_env.tick_irq, _SsSeqCalledTbyTEveryOther);
        }
    }
    ExitCriticalSection();
}

void SsStart(void)
{
    _SsStart(1);
}

void SsStart2(void)
{
    _SsStart(0);
}

static void _SsTrapIntrVSync(void)
{
    if (_snd_seq_tick_env.vsync_callback) {
        _snd_seq_tick_env.vsync_callback();
    }
    _snd_seq_tick_env.callback();
}

static void _SsSeqCalledTbyTEveryOther(void)
{
    static long odd = 0;

    if (odd == 0) {
        odd = 1;
    } else {
        odd = 0;
        _snd_seq_tick_env.callback();
    }
}
