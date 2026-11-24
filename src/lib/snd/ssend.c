/* Stop the sound sequencer tick interrupt and restore any chained VBlank handler. */

#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include <libsnd.h>

#include "etc_int.h"
#include "snd_int.h"

void SsEnd(void)
{
    if (_snd_seq_tick_env.manual) {
        return;
    }
    _snd_seq_tick_env.every_other = 0;
    if (_snd_seq_tick_env.tick_irq == SND_TICK_IRQ_NONE) {
        return;
    }
    EnterCriticalSection();
    if (_snd_seq_tick_env.vsync_tick) {
        VSyncCallback(NULL);
        _snd_seq_tick_env.vsync_tick = 0;
    } else if (_snd_seq_tick_env.tick_irq == PSX_IRQ_VBLANK) {
        InterruptCallback(PSX_IRQ_VBLANK, _snd_seq_tick_env.vsync_callback);
        _snd_seq_tick_env.vsync_callback = NULL;
    } else {
        InterruptCallback(PSX_IRQ_RCNT2, NULL);
    }
    ExitCriticalSection();
    _snd_seq_tick_env.tick_irq = SND_TICK_IRQ_NONE;
}
