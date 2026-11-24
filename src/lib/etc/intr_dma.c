/*
 * LIBETC DMA interrupt dispatcher and per-channel callback registration.
 */
#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include "etc_int.h"

#define DMA_CALLBACK_SLOTS      8

static void trapIntrDMA(void);
static IntrCallback setIntrDMA(long ch, IntrCallback func);
static void memclr(long *dst, long nwords);

static volatile u_long *DICR = (volatile u_long *)DMA_DICR_ADDR;
static IntrCallback dma_callbacks[DMA_CALLBACK_SLOTS] = {0};
static volatile struct DmaChReg *DMA_CH = (volatile struct DmaChReg *)DMA0_MADR_ADDR;

IntrCallbackSetter startIntrDMA(void)
{
    memclr((long *)dma_callbacks, DMA_CALLBACK_SLOTS);
    *DICR = 0;
    InterruptCallback(PSX_IRQ_DMA, trapIntrDMA);
    return setIntrDMA;
}

static void trapIntrDMA(void)
{
    u_long mask;
    long ch;

    while ((mask = (*DICR >> DICR_IRQ_FLAG_SHIFT) & DICR_IRQ_FLAG_MASK) != 0) {
        for (ch = 0; mask && ch < DMA_CH_COUNT; ch++, mask >>= 1) {
            if (mask & 1) {
                /* keep control bits and acknowledge only this channel's write-1-to-clear flag */
                *DICR &= (1 << (ch + DICR_IRQ_FLAG_SHIFT)) | DICR_CTRL_MASK;
                if (dma_callbacks[ch]) {
                    (*dma_callbacks[ch])();
                }
            }
        }
    }
    if ((*DICR & DICR_FLAG_MASK) == DICR_MASTER_FLAG || (*DICR & DICR_BUS_ERROR)) {
        printf("DMA bus error: code=%08x\n", *DICR);
        for (ch = 0; ch < DMA_CH_COUNT; ch++) {
            printf("MADR[%d]=%08x\n", ch, DMA_CH[ch].madr);
        }
    }
}

static IntrCallback setIntrDMA(long ch, IntrCallback func)
{
    IntrCallback old = dma_callbacks[ch];

    if (func != old) {
        if (func) {
            dma_callbacks[ch] = func;
            *DICR = (*DICR & DICR_CTRL_MASK) | DICR_MASTER_EN | (1 << (ch + DICR_IRQ_EN_SHIFT));
        } else {
            dma_callbacks[ch] = 0;
            *DICR = ((*DICR & DICR_CTRL_MASK) | DICR_MASTER_EN) & ~(1 << (ch + DICR_IRQ_EN_SHIFT));
        }
    }
    return old;
}

static void memclr(long *dst, long nwords)
{
    while (nwords--) {
        *dst++ = 0;
    }
}
