/* Low-level LIBSND initialization: resets SPU registers, voice manager, and sequence state. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"
#include "snd_vm_int.h"
#include "vm_init.h"

long _snd_vmask = 0;

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static long snd_id[2] = { 0x25107350, 0x0043539b };

#define SPU_COMMON_REG_COUNT 16

static unsigned short voice_init[SPU_VREG_COUNT] = {
    0x0000, 0x0000, 0x1000, 0x3000, 0x00bf, 0x0000, 0x0000, 0x0000
};

static unsigned short common_init[SPU_COMMON_REG_COUNT] = {
    0x3fff, 0x3fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

_SsFCALL SsFCALL;
SsMarkCallbackProc _SsMarkCallback[SND_SEQ_SEP_MAX][SND_SEQ_TRACK_MAX];
SndSeqScore *_ss_score[SND_SEQ_SEP_MAX];
long VBLANK_MINUS;
unsigned long _snd_openflag;
short _snd_seq_s_max;
short _snd_seq_t_max;
long _snd_ev_flag;

void _SsInit(void)
{
    int i;
    int j;
    unsigned short *sreg;

    sreg = (unsigned short *)SPU_REG_BASE;
    for (i = 0; i < SPU_VOICE_COUNT; i++) {
        for (j = 0; j < SPU_VREG_COUNT; j++) {
            *sreg++ = voice_init[j];
        }
    }

    sreg = (unsigned short *)SPU_REG_BASE + SPU_REG_MVOL_L;
    for (i = 0; i < SPU_COMMON_REG_COUNT; i++) {
        *sreg++ = common_init[i];
    }

    _SsVmInit(SPU_VOICE_COUNT);

    for (j = 0; j < SND_SEQ_SEP_MAX; j++) {
        for (i = SND_SEQ_TRACK_MAX - 1; i >= 0; i--) {
            _SsMarkCallback[j][i] = NULL;
        }
    }

    VBLANK_MINUS = 60;
    _snd_openflag = 0;
    _snd_ev_flag = 0;
}
