/*
 * Waiting for vertical blank, and the vertical and horizontal blank counts.
 */
#include <sys/types.h>
#include <stdio.h>
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include "etc_int.h"

static void v_wait(long target_vcount, long timeout);

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static long etc_id[2] = {0x25067350, 0x0043529b};
static volatile u_long *GPU_STATUS = (volatile u_long *)GPU_STATUS_ADDR;
static volatile u_long *RCNT1_COUNT = (volatile u_long *)RCNT1_COUNT_ADDR;
volatile long Hcount = 0;
static long last_vcount = 0;

int VSync(int mode)
{
    /* Preserve the entry GPU status read even in query modes. */
    u_long status = *GPU_STATUS;
    volatile u_long hsample;
    long hcount;

    /* Require two consecutive timer reads to agree. */
    do {
        hsample = *RCNT1_COUNT;
    } while (hsample != *RCNT1_COUNT);
    hcount = (hsample - Hcount) & 0xffff;

    if (mode < 0) {
        return Vcount;
    }
    if (mode != 1) {
        long target_vcount = mode > 0 ? last_vcount + (mode - 1) : last_vcount;
        mode = mode > 0 ? mode - 1 : 0;
        v_wait(target_vcount, mode);
        status = *GPU_STATUS;
        v_wait(Vcount + 1, 1);
        if (status & GPUSTAT_INTERLACE_ENABLED) {
            while (!((status ^ *GPU_STATUS) & GPUSTAT_INTERLACE_FIELD)) {
            }
        }
        last_vcount = Vcount;
        do {
            Hcount = *RCNT1_COUNT;
        } while (Hcount != *RCNT1_COUNT);
    }
    return hcount;
}

static void v_wait(long target_vcount, long timeout)
{
    volatile u_long left = (u_long)timeout << 15;

    while (Vcount < target_vcount) {
        if (--left == (u_long)-1) {
            puts("VSync: timeout\n");
            ChangeClearPAD(0);
            ChangeClearRCnt(RCNT_VBLANK, 0);
            return;
        }
    }
}
