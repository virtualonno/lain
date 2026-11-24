/* SPU hardware layer: initialization, Sound RAM transfers by DMA and I/O, register access, bus delay. */
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <kernel.h>
#include <libapi.h>
#include <libspu.h>

#include "spu_int.h"

/* gSpuInitBlock's place in SPU RAM in 8-byte units (byte 0x1000), and the transfer block sizes. */
#define SPU_INIT_BLOCK_TSA      0x0200
#define SPU_IO_BLOCK_SIZE       64
#define SPU_DMA_BLOCK_SIZE      64

#define DMA_DPCR_SPU_MASK       0x00070000
#define DMA_DPCR_SPU_PRI_HIGH   0x00030000
#define DMA_DPCR_SPU_PRI_LOW    0x00050000
#define DMA_DPCR_SPU_INIT       0x000b0000      /* DMA4 enable | priority 3 */
#define DMA_BCR_BLOCK_SIZE      0x10            /* 16 words = 64 bytes per block */
#define DMA_CHCR_SPU_READ       0x01000200      /* SPU -> RAM, block sync mode, start */
#define DMA_CHCR_SPU_WRITE      0x01000201      /* RAM -> SPU, block sync mode, start */
#define SPU_DELAY_TIMING_MASK   0x0f000000
#define SPU_DELAY_WRITE         0x20000000
#define SPU_DELAY_READ          0x22000000

/* gSpuDmaMode direction values */
#define SPU_DMA_WRITE           0
#define SPU_DMA_READ            1

static void _spu_writeByIO(unsigned char *addr, unsigned long size);
static void _spu_FsetDelayW(void);
static void _spu_FsetDelayR(void);
void _spu_Fw1ts(void);

volatile unsigned short *_spu_RXX = (volatile unsigned short *)SPU_REG_BASE;
static volatile unsigned long *DMA4_MADR = (volatile unsigned long *)DMA4_MADR_ADDR;
static volatile unsigned long *DMA4_BCR = (volatile unsigned long *)DMA4_BCR_ADDR;
static volatile unsigned long *DMA4_CHCR = (volatile unsigned long *)DMA4_CHCR_ADDR;
static volatile unsigned long *DMA_DPCR = (volatile unsigned long *)DMA_DPCR_ADDR;
static volatile unsigned long *SPU_DELAY = (volatile unsigned long *)SPU_DELAY_ADDR;

unsigned short _spu_tsa = 0;
long _spu_transMode = SPU_TRANSFER_BY_DMA;
long _spu_addrMode = 0;
/* Sound RAM address granularity: 8-byte units (shift 3, alignment mask 7) */
long _spu_mem_mode = 2;
long _spu_mem_mode_plus = 3;
long _spu_mem_mode_unit = 8;
long _spu_mem_mode_unitM = 7;
long _spu_inTransfer = 1;
volatile SpuTransferCallbackProc _spu_transferCallback = NULL;
volatile SpuIRQCallbackProc _spu_IRQCallback = NULL;

/* A cold _spu_init writes this to SPU_INIT_BLOCK_TSA and points every voice at it. */
static unsigned char gSpuInitBlock[16] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};
static long gSpuDmaMode = SPU_DMA_WRITE;
static unsigned long gSpuDmaAddr = 0;
static unsigned long gSpuDmaBlocks = 0;

volatile unsigned short _spu_RQ[SPU_RQ_COUNT];

long _spu_init(long mode)
{
    unsigned long count;
    int i;

    *DMA_DPCR |= DMA_DPCR_SPU_INIT;
    _spu_transMode = SPU_TRANSFER_BY_DMA;
    _spu_addrMode = 0;
    _spu_tsa = 0;
    _spu_RXX[SPU_REG_MVOL_L] = 0;
    _spu_RXX[SPU_REG_MVOL_R] = 0;
    _spu_RXX[SPU_REG_SPUCNT] = 0;
    _spu_Fw1ts();
    _spu_RXX[SPU_REG_MVOL_L] = 0;
    _spu_RXX[SPU_REG_MVOL_R] = 0;
    count = 0;
    while (_spu_RXX[SPU_REG_SPUSTAT] & SPUSTAT_MASK) {
        if (++count > SPU_WAIT_TIMEOUT) {
            printf("SPU:T/O [%s]\n", "wait (reset)");
            break;
        }
    }
    _spu_mem_mode = 2;
    _spu_mem_mode_plus = 3;
    _spu_mem_mode_unit = 8;
    _spu_mem_mode_unitM = 7;
    _spu_RXX[SPU_REG_TCTRL] = SPU_TCTRL_NORMAL;
    _spu_RXX[SPU_REG_RVOL_L] = 0;
    _spu_RXX[SPU_REG_RVOL_R] = 0;
    _spu_RXX[SPU_REG_KOFF_L] = 0xffff;
    _spu_RXX[SPU_REG_KOFF_H] = 0xffff;
    _spu_RXX[SPU_REG_EON_L] = 0;
    _spu_RXX[SPU_REG_EON_H] = 0;
    for (i = 0; i < SPU_RQ_COUNT; i++) {
        _spu_RQ[i] = 0;
    }
    if (mode == SPU_INIT_COLD) {
        _spu_tsa = SPU_INIT_BLOCK_TSA;
        _spu_RXX[SPU_REG_PMON_L] = 0;
        _spu_RXX[SPU_REG_PMON_H] = 0;
        _spu_RXX[SPU_REG_NON_L] = 0;
        _spu_RXX[SPU_REG_NON_H] = 0;
        _spu_RXX[SPU_REG_CDVOL_L] = 0;
        _spu_RXX[SPU_REG_CDVOL_R] = 0;
        _spu_RXX[SPU_REG_EXTVOL_L] = 0;
        _spu_RXX[SPU_REG_EXTVOL_R] = 0;
        _spu_writeByIO(gSpuInitBlock, sizeof(gSpuInitBlock));
        for (i = 0; i < SPU_VOICE_COUNT; i++) {
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_VOL_L] = 0;
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_VOL_R] = 0;
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_PITCH] = 0x3fff;
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_START_ADDR] = SPU_INIT_BLOCK_TSA;
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_ADSR1] = 0;
            _spu_RXX[i * SPU_VREG_COUNT + SPU_VREG_ADSR2] = 0;
        }
        _spu_RXX[SPU_REG_KON_L] = 0xffff;
        _spu_RXX[SPU_REG_KON_H] = 0xff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_RXX[SPU_REG_KOFF_L] = 0xffff;
        _spu_RXX[SPU_REG_KOFF_H] = 0xff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
    }
    _spu_inTransfer = 1;
    _spu_RXX[SPU_REG_SPUCNT] = SPUCNT_ENABLE | SPUCNT_UNMUTE;
    _spu_transferCallback = NULL;
    _spu_IRQCallback = NULL;
    return SPU_SUCCESS;
}

static void _spu_writeByIO(unsigned char *addr, unsigned long size)
{
    unsigned short status;
    unsigned short spucnt;
    unsigned short *src;
    unsigned long count;
    long block;
    long i;

    status = _spu_RXX[SPU_REG_SPUSTAT] & SPUSTAT_MASK;
    _spu_RXX[SPU_REG_TSA] = _spu_tsa;
    src = (unsigned short *)addr;
    _spu_Fw1ts();
    while (size != 0) {
        block = SPU_IO_BLOCK_SIZE;
        if (size <= SPU_IO_BLOCK_SIZE) {
            block = size;
        }
        for (i = 0; i < block; i += 2) {
            _spu_RXX[SPU_REG_DATA] = *src++;
        }
        spucnt = _spu_RXX[SPU_REG_SPUCNT];
        spucnt = (spucnt & ~SPUCNT_TRANS_MASK) | SPUCNT_TRANS_IO;
        _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        _spu_Fw1ts();
        count = 0;
        while (_spu_RXX[SPU_REG_SPUSTAT] & SPUSTAT_DMA_BUSY) {
            if (++count > SPU_WAIT_TIMEOUT) {
                printf("SPU:T/O [%s]\n", "wait (wrdy H -> L)");
                break;
            }
        }
        _spu_Fw1ts();
        size -= block;
        _spu_Fw1ts();
    }
    spucnt = _spu_RXX[SPU_REG_SPUCNT];
    spucnt &= ~SPUCNT_TRANS_MASK;
    _spu_RXX[SPU_REG_SPUCNT] = spucnt;
    count = 0;
    while ((_spu_RXX[SPU_REG_SPUSTAT] & SPUSTAT_MASK) != status) {
        if (++count > SPU_WAIT_TIMEOUT) {
            printf("SPU:T/O [%s]\n", "wait (dmaf clear/W)");
            break;
        }
    }
}

void _spu_FiDMA(void)
{
    unsigned long count;

    if (gSpuDmaMode == SPU_DMA_WRITE) {
        _spu_Fw1ts();
    }
    _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_TRANS_MASK;
    count = 0;
    while (_spu_RXX[SPU_REG_SPUCNT] & SPUCNT_TRANS_MASK) {
        if (++count > SPU_WAIT_TIMEOUT) {
            break;
        }
    }
    if (_spu_transferCallback) {
        (*_spu_transferCallback)();
    } else {
        DeliverEvent(HwSPU, EvSpCOMP);
    }
}

void _spu_Fr_(unsigned long addr, unsigned short tsa, unsigned long blocks)
{
    _spu_RXX[SPU_REG_TSA] = tsa;
    _spu_Fw1ts();
    _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_TRANS_DMAR;
    _spu_Fw1ts();
    _spu_FsetDelayR();
    *DMA4_MADR = addr;
    *DMA4_BCR = (blocks << DMA_BCR_COUNT_SHIFT) | DMA_BCR_BLOCK_SIZE;
    gSpuDmaMode = SPU_DMA_READ;
    *DMA4_CHCR = DMA_CHCR_SPU_READ;
}

long _spu_t(long mode, ...)
{
    va_list ap;
    unsigned long count;
    unsigned long param;
    unsigned short spucnt;
    unsigned long chcr;

    va_start(ap, mode);
    switch (mode) {
    case SPU_TRANS_ADDR:
        param = va_arg(ap, unsigned long);
        _spu_RXX[SPU_REG_TSA] = _spu_tsa = param >> _spu_mem_mode_plus;
        break;
    case SPU_TRANS_WRITE:
        gSpuDmaMode = SPU_DMA_WRITE;
        count = 0;
        while (_spu_RXX[SPU_REG_TSA] != _spu_tsa) {
            if (++count > SPU_WAIT_TIMEOUT) {
                va_end(ap);
                return SPU_TRANS_TIMEOUT;
            }
        }
        _spu_RXX[SPU_REG_SPUCNT] = (_spu_RXX[SPU_REG_SPUCNT] & ~SPUCNT_TRANS_MASK) | SPUCNT_TRANS_DMAW;
        break;
    case SPU_TRANS_READ:
        gSpuDmaMode = SPU_DMA_READ;
        count = 0;
        while (_spu_RXX[SPU_REG_TSA] != _spu_tsa) {
            if (++count > SPU_WAIT_TIMEOUT) {
                va_end(ap);
                return SPU_TRANS_TIMEOUT;
            }
        }
        _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_TRANS_DMAR;
        break;
    case SPU_TRANS_START:
        spucnt = (gSpuDmaMode == SPU_DMA_READ) ? SPUCNT_TRANS_DMAR : SPUCNT_TRANS_DMAW;
        count = 0;
        while ((_spu_RXX[SPU_REG_SPUCNT] & SPUCNT_TRANS_MASK) != spucnt) {
            if (++count > SPU_WAIT_TIMEOUT) {
                va_end(ap);
                return SPU_TRANS_TIMEOUT;
            }
        }
        if (gSpuDmaMode == SPU_DMA_READ) {
            _spu_FsetDelayR();
        } else {
            _spu_FsetDelayW();
        }
        param = va_arg(ap, unsigned long);
        gSpuDmaAddr = param;
        param = va_arg(ap, unsigned long);
        gSpuDmaBlocks = (param / SPU_DMA_BLOCK_SIZE) + ((param % SPU_DMA_BLOCK_SIZE) != 0);
        *DMA4_MADR = gSpuDmaAddr;
        *DMA4_BCR = (gSpuDmaBlocks << DMA_BCR_COUNT_SHIFT) | DMA_BCR_BLOCK_SIZE;
        if (gSpuDmaMode == SPU_DMA_READ) {
            chcr = DMA_CHCR_SPU_READ;
        } else {
            chcr = DMA_CHCR_SPU_WRITE;
        }
        *DMA4_CHCR = chcr;
        break;
    }
    va_end(ap);
    return SPU_SUCCESS;
}

unsigned long _spu_Fw(unsigned char *addr, unsigned long size)
{
    if (_spu_transMode == SPU_TRANSFER_BY_DMA) {
        _spu_t(SPU_TRANS_ADDR, _spu_tsa << _spu_mem_mode_plus);
        _spu_t(SPU_TRANS_WRITE);
        _spu_t(SPU_TRANS_START, addr, size);
    } else {
        _spu_writeByIO(addr, size);
    }
    return size;
}

unsigned long _spu_Fr(unsigned char *addr, unsigned long size)
{
    _spu_t(SPU_TRANS_ADDR, _spu_tsa << _spu_mem_mode_plus);
    _spu_t(SPU_TRANS_READ);
    _spu_t(SPU_TRANS_START, addr, size);
    return size;
}

void _spu_FsetRXX(long reg, unsigned long value, long mode)
{
    if (mode == SPU_FSETRXX_RAW) {
        _spu_RXX[reg] = value;
    } else {
        _spu_RXX[reg] = value >> _spu_mem_mode_plus;
    }
}

unsigned long _spu_FsetRXXa(long reg, unsigned long value)
{
    unsigned short scaled;

    if (_spu_mem_mode != 0 && value % _spu_mem_mode_unit != 0) {
        value += _spu_mem_mode_unit;
        value &= ~_spu_mem_mode_unitM;
    }
    scaled = value >> _spu_mem_mode_plus;
    switch (reg) {
    case SPU_CHECK:
        return scaled;
    case SPU_DIAG:
        return value;
    default:
        _spu_RXX[reg] = scaled;
        return value;
    }
}

unsigned long _spu_FgetRXXa(long reg, long mode)
{
    unsigned short value = _spu_RXX[reg];

    if (mode == -1) {
        return value;
    }
    return value << _spu_mem_mode_plus;
}

void _spu_FsetPCR(long high_priority)
{
    *DMA_DPCR &= ~DMA_DPCR_SPU_MASK;
    if (high_priority) {
        *DMA_DPCR |= DMA_DPCR_SPU_PRI_HIGH;
    } else {
        *DMA_DPCR |= DMA_DPCR_SPU_PRI_LOW;
    }
}

static void _spu_FsetDelayW(void)
{
    *SPU_DELAY = (*SPU_DELAY & ~SPU_DELAY_TIMING_MASK) | SPU_DELAY_WRITE;
}

static void _spu_FsetDelayR(void)
{
    *SPU_DELAY = (*SPU_DELAY & ~SPU_DELAY_TIMING_MASK) | SPU_DELAY_READ;
}

/* Busy-wait loop between SPU register accesses */
void _spu_Fw1ts(void)
{
    volatile long i;
    volatile unsigned long acc;

    acc = 13;
    for (i = 0; i < 60; i++) {
        acc *= 13;
    }
}
