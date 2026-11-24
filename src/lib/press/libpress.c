/*
 * MDEC image decompression control, DMA transfer, and environment routines.
 */
#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include <libpress.h>
#include "etc_int.h"

#define DMA_DPCR_MDEC_EN	0x88		/* enable DMA0 (MDEC in) and DMA1 (MDEC out) */
#define DMA_CHCR_MDEC_IN	0x01000201	/* RAM -> MDEC, block sync mode, start */
#define DMA_CHCR_MDEC_OUT	0x01000200	/* MDEC -> RAM, block sync mode, start */
#define MDEC_DMA_BLOCK_WORDS	0x20

#define MDEC_CMD_SET_IQ_YC	0x40000001	/* luminance and chrominance quantization tables follow */
#define MDEC_CMD_SET_IDCT	0x60000000	/* IDCT table follows */
#define MDEC_CMD_DEPTH16	0x08000000	/* 15-bit output when set, 24-bit when clear */
#define MDEC_CMD_STP		0x02000000	/* sets bit 15 of each 15-bit pixel */
#define MDEC_TABLE_WORDS	32

#define DECDCT_MODE_24BIT	1
#define DECDCT_MODE_STP		2

#define MDEC_CTRL_RESET		0x80000000
#define MDEC_CTRL_DMA_EN	0x60000000	/* data-in and data-out DMA requests */
#define MDEC_STAT_BUSY		0x20000000
#define MDEC_STAT_BUSY_SHIFT		29
#define MDEC_STAT_OUT_SIGNED_SHIFT	24

#define MDEC_SYNC_TIMEOUT	0x100000

static void MDEC_reset(int mode);
static void MDEC_in(u_long *addr, u_long size);
static void MDEC_out(u_long *addr, u_long size);
static int MDEC_in_sync(void);
static int MDEC_out_sync(void);
static u_long MDEC_status(void);
static int timeout(char *func);

/* the MDEC's set-quantization command and its two tables (luminance, chrominance) */
static struct MdecIqPacket {
    u_long cmd;
    u_char iq_y[64];
    u_char iq_c[64];
} mdec_iq = {
    MDEC_CMD_SET_IQ_YC,
    {
        2, 16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27,
        27, 27, 26, 26, 26, 26, 27, 27, 27, 29, 29, 29, 34, 34, 34, 29,
        29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38, 37, 35, 35, 34,
        35, 38, 38, 40, 40, 40, 48, 48, 46, 46, 56, 56, 58, 69, 69, 83,
    },
    {
        2, 16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27,
        27, 27, 26, 26, 26, 26, 27, 27, 27, 29, 29, 29, 34, 34, 34, 29,
        29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38, 37, 35, 35, 34,
        35, 38, 38, 40, 40, 40, 48, 48, 46, 46, 56, 56, 58, 69, 69, 83,
    },
};

/* the MDEC's set-IDCT-table command and its coefficients */
static struct MdecIdctPacket {
    u_long cmd;
    short dct[64];
} mdec_dct = {
    MDEC_CMD_SET_IDCT,
    {
        23170, 23170, 23170, 23170, 23170, 23170, 23170, 23170,
        32138, 27245, 18204, 6392, -6393, -18205, -27246, -32139,
        30273, 12539, -12540, -30274, -30274, -12540, 12539, 30273,
        27245, -6393, -32139, -18205, 18204, 32138, 6392, -27246,
        23170, -23171, -23171, 23170, 23170, -23171, -23171, 23170,
        18204, -32139, 6392, 27245, -27246, -6393, 32138, -18205,
        12539, -30274, 30273, -12540, -12540, 30273, -30274, 12539,
        6392, -18205, 27245, -32139, 32138, -27246, 18204, -6393,
    },
};

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static long press_id[2] = {0x250e7350, 0x0043539b};

static volatile u_long *DMA0_MADR = (u_long *)DMA0_MADR_ADDR;
static volatile u_long *DMA0_BCR = (u_long *)DMA0_BCR_ADDR;
static volatile u_long *DMA0_CHCR = (u_long *)DMA0_CHCR_ADDR;
static volatile u_long *DMA1_MADR = (u_long *)DMA1_MADR_ADDR;
static volatile u_long *DMA1_BCR = (u_long *)DMA1_BCR_ADDR;
static volatile u_long *DMA1_CHCR = (u_long *)DMA1_CHCR_ADDR;
/* the six DMA2 and DMA3 pointers, 24 bytes: nothing reads them */
static volatile u_long *DMA2_MADR = (u_long *)DMA2_MADR_ADDR;
static volatile u_long *DMA2_BCR = (u_long *)DMA2_BCR_ADDR;
static volatile u_long *DMA2_CHCR = (u_long *)DMA2_CHCR_ADDR;
static volatile u_long *DMA3_MADR = (u_long *)DMA3_MADR_ADDR;
static volatile u_long *DMA3_BCR = (u_long *)DMA3_BCR_ADDR;
static volatile u_long *DMA3_CHCR = (u_long *)DMA3_CHCR_ADDR;
static volatile u_long *MDEC_DATA = (u_long *)MDEC_DATA_ADDR;
static volatile u_long *MDEC_STAT = (u_long *)MDEC_STAT_ADDR;
static volatile u_long *DMA_DPCR = (u_long *)DMA_DPCR_ADDR;

void DecDCTReset(int mode)
{
    if (mode == 0) {
        ResetCallback();
    }
    MDEC_reset(mode);
}

DECDCTENV *DecDCTGetEnv(DECDCTENV *env)
{
    u_long *src, *dst;
    int words;

    dst = (u_long *)env->iq_y;
    src = (u_long *)mdec_iq.iq_y;
    words = sizeof(env->iq_y) / sizeof(u_long);
    while (words--) {
        *dst++ = *src++;
    }
    dst = (u_long *)env->iq_c;
    src = (u_long *)mdec_iq.iq_c;
    words = sizeof(env->iq_c) / sizeof(u_long);
    while (words--) {
        *dst++ = *src++;
    }
    dst = (u_long *)env->dct;
    src = (u_long *)mdec_dct.dct;
    words = sizeof(env->dct) / sizeof(u_long);
    while (words--) {
        *dst++ = *src++;
    }
    return env;
}

DECDCTENV *DecDCTPutEnv(DECDCTENV *env)
{
    u_long *src, *dst;
    int words;

    dst = (u_long *)mdec_iq.iq_y;
    src = (u_long *)env->iq_y;
    words = sizeof(env->iq_y) / sizeof(u_long);
    while (words--) {
        *dst++ = *src++;
    }
    dst = (u_long *)mdec_iq.iq_c;
    src = (u_long *)env->iq_c;
    words = sizeof(env->iq_c) / sizeof(u_long);
    while (words--) {
        *dst++ = *src++;
    }
    MDEC_in((u_long *)&mdec_iq, MDEC_TABLE_WORDS);
    MDEC_in((u_long *)&mdec_dct, MDEC_TABLE_WORDS);
    return env;
}

int DecDCTBufSize(u_long *bs)
{
    return *(u_short *)bs;
}

void DecDCTin(u_long *buf, int mode)
{
    if (mode & DECDCT_MODE_24BIT) {
        *buf &= ~MDEC_CMD_DEPTH16;
    } else {
        *buf |= MDEC_CMD_DEPTH16;
    }
    if (mode & DECDCT_MODE_STP) {
        *buf |= MDEC_CMD_STP;
    } else {
        *buf &= ~MDEC_CMD_STP;
    }
    MDEC_in(buf, *(u_short *)buf);
}

void DecDCTout(u_long *buf, int size)
{
    MDEC_out(buf, size);
}

int DecDCTinSync(int mode)
{
    int ret;

    if (mode == 0) {
        ret = MDEC_in_sync();
    } else {
        ret = (MDEC_status() >> MDEC_STAT_BUSY_SHIFT) & 1;
    }
    return ret;
}

int DecDCToutSync(int mode)
{
    int ret;

    if (mode == 0) {
        ret = MDEC_out_sync();
    } else {
        ret = (MDEC_status() >> MDEC_STAT_OUT_SIGNED_SHIFT) & 1;
    }
    return ret;
}

int DecDCTinCallback(void (*func)())
{
    return (int)DMACallback(DMA_MDEC_IN, func);
}

int DecDCToutCallback(void (*func)())
{
    return (int)DMACallback(DMA_MDEC_OUT, func);
}

static void MDEC_reset(int mode)
{
    switch (mode) {
    case 0:
        *MDEC_STAT = MDEC_CTRL_RESET;
        *DMA0_CHCR = 0;
        *DMA1_CHCR = 0;
        *MDEC_STAT = MDEC_CTRL_DMA_EN;
        MDEC_in((u_long *)&mdec_iq, MDEC_TABLE_WORDS);
        MDEC_in((u_long *)&mdec_dct, MDEC_TABLE_WORDS);
        break;
    case 1:
        *MDEC_STAT = MDEC_CTRL_RESET;
        *DMA0_CHCR = 0;
        *DMA1_CHCR = 0;
        *DMA1_CHCR;
        *MDEC_STAT = MDEC_CTRL_DMA_EN;
        break;
    default:
        printf("MDEC_rest:bad option(%d)\n", mode);
        break;
    }
}

static void MDEC_in(u_long *addr, u_long size)
{
    MDEC_in_sync();
    *DMA_DPCR |= DMA_DPCR_MDEC_EN;
    *DMA0_MADR = (u_long)(addr + 1);
    *DMA0_BCR = ((size / MDEC_DMA_BLOCK_WORDS) << DMA_BCR_COUNT_SHIFT) | MDEC_DMA_BLOCK_WORDS;
    *MDEC_DATA = *addr;
    *DMA0_CHCR = DMA_CHCR_MDEC_IN;
}

static void MDEC_out(u_long *addr, u_long size)
{
    MDEC_out_sync();
    *DMA_DPCR |= DMA_DPCR_MDEC_EN;
    *DMA1_CHCR = 0;
    *DMA1_MADR = (u_long)addr;
    *DMA1_BCR = ((size / MDEC_DMA_BLOCK_WORDS) << DMA_BCR_COUNT_SHIFT) | MDEC_DMA_BLOCK_WORDS;
    *DMA1_CHCR = DMA_CHCR_MDEC_OUT;
}

static int MDEC_in_sync(void)
{
    volatile long timeout_cnt = MDEC_SYNC_TIMEOUT;

    while (*MDEC_STAT & MDEC_STAT_BUSY) {
        if (--timeout_cnt == -1) {
            timeout("MDEC_in_sync");
            return -1;
        }
    }
    return 0;
}

static int MDEC_out_sync(void)
{
    volatile long timeout_cnt = MDEC_SYNC_TIMEOUT;

    while (*DMA1_CHCR & DMA_CHCR_BUSY) {
        if (--timeout_cnt == -1) {
            timeout("MDEC_out_sync");
            return -1;
        }
    }
    return 0;
}

static u_long MDEC_status(void)
{
    return *MDEC_STAT;
}

static int timeout(char *func)
{
    printf("%s timeout:\n", func);
    *MDEC_STAT = MDEC_CTRL_RESET;
    *DMA0_CHCR = 0;
    *DMA1_CHCR = 0;
    *DMA1_CHCR;
    *MDEC_STAT = MDEC_CTRL_DMA_EN;
    return 0;
}
