#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include <libspu.h>

#include "spu_int.h"

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static long spu_id[2] = { 0x25117350, 0x0043539b };

long _spu_EVdma = 0;
unsigned long _spu_keystat = 0;
long _spu_trans_mode = SPU_TRANSFER_BY_DMA;
long _spu_rev_flag = SPU_OFF;
long _spu_rev_reserve_wa = SPU_OFF;
long _spu_rev_offsetaddr = 0;
SpuReverbAttr _spu_rev_attr = { 0, SPU_REV_MODE_OFF, { 0, 0 }, 0, 0 };
volatile unsigned long _spu_RQvoice = 0;
volatile unsigned long _spu_RQmask = 0;
unsigned short _spu_voice_centerNote[SPU_VOICE_COUNT] = {
    0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
    0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
    0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
    0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
};
unsigned long _spu_env = 0;

void _SpuInit(long mode)
{
    long voice;

    ResetCallback();
    _spu_init(mode);
    if (mode == SPU_INIT_COLD) {
        for (voice = 0; voice < SPU_VOICE_COUNT; voice++) {
            _spu_voice_centerNote[voice] = 0xc000;
        }
    }
    SpuStart();
    _spu_rev_flag = SPU_OFF;
    _spu_rev_reserve_wa = SPU_OFF;
    _spu_rev_attr.mode = SPU_REV_MODE_OFF;
    _spu_rev_attr.depth.left = 0;
    _spu_rev_attr.depth.right = 0;
    _spu_rev_attr.delay = 0;
    _spu_rev_attr.feedback = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr[SPU_REV_MODE_OFF];
    _spu_FsetRXX(SPU_REG_MBASE, _spu_rev_offsetaddr, SPU_FSETRXX_RAW);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = NULL;
    _spu_trans_mode = SPU_TRANSFER_BY_DMA;
    _spu_transMode = SPU_TRANSFER_BY_DMA;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
}

long _spu_isCalled = 0;

void SpuStart(void)
{
    if (_spu_isCalled == 0) {
        _spu_isCalled = 1;
        EnterCriticalSection();
        _SpuDataCallback(_spu_FiDMA);
        _spu_EVdma = OpenEvent(HwSPU, EvSpCOMP, EvMdNOINTR, NULL);
        EnableEvent(_spu_EVdma);
        ExitCriticalSection();
    }
}
