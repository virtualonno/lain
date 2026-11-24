#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <libspu.h>

#include "spu_int.h"

/* With reverb off, only the last 128 bytes of SPU RAM are cleared. */
#define SPU_REV_OFF_CLEAR_ADDR  0xfff0
#define SPU_REV_OFF_CLEAR_SIZE  0x10

long SpuClearReverbWorkArea(long mode)
{
    unsigned long size, addr, chunk_size;
    unsigned long rev_mode = mode;
    long more;
    long previous_trans_mode;
    long restore_trans_mode = 0;
    volatile SpuTransferCallbackProc saved_callback = NULL;

    if (rev_mode >= SPU_REV_MODE_MAX ||
        _SpuIsInAllocateArea_(_spu_rev_startaddr[rev_mode])) {
        return SPU_ERROR;
    }
    if (rev_mode == SPU_REV_MODE_OFF) {
        size = SPU_REV_OFF_CLEAR_SIZE << _spu_mem_mode_plus;
        addr = SPU_REV_OFF_CLEAR_ADDR << _spu_mem_mode_plus;
    } else {
        size = (SPU_RAM_UNITS - _spu_rev_startaddr[rev_mode]) << _spu_mem_mode_plus;
        addr = _spu_rev_startaddr[rev_mode] << _spu_mem_mode_plus;
    }
    previous_trans_mode = _spu_transMode;
    if (previous_trans_mode == SPU_TRANSFER_BY_IO) {
        _spu_transMode = SPU_TRANSFER_BY_DMA;
        restore_trans_mode = 1;
    }
    more = 1;
    if (_spu_transferCallback) {
        saved_callback = _spu_transferCallback;
        _spu_transferCallback = NULL;
    }
    do {
        if (size > SPU_ZEROBUF_SIZE) {
            chunk_size = SPU_ZEROBUF_SIZE;
        } else {
            chunk_size = size;
            more = 0;
        }
        _spu_t(SPU_TRANS_ADDR, addr);
        _spu_t(SPU_TRANS_WRITE);
        _spu_t(SPU_TRANS_START, _spu_zerobuf, chunk_size);
        WaitEvent(_spu_EVdma);
        size -= SPU_ZEROBUF_SIZE;
        addr += SPU_ZEROBUF_SIZE;
    } while (more);
    if (restore_trans_mode) {
        _spu_transMode = previous_trans_mode;
    }
    if (saved_callback) {
        _spu_transferCallback = saved_callback;
    }
    return SPU_SUCCESS;
}
