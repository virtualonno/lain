/* PlayStation hardware root counter control (SetRCnt, GetRCnt, StartRCnt,
 * StopRCnt, ResetRCnt). */

#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include "psx_hw.h"

#define RCNT_MODE_BASE  (RCNT_MODE_RESET_TARGET | RCNT_MODE_REPEAT_IRQ)

static volatile struct IntrReg *intrRegs = (struct IntrReg *)INTR_STAT_ADDR;
static volatile struct RCntReg *rcntRegs = (struct RCntReg *)RCNT_REG_BASE;
static long rcntIntrMask[RCNT_COUNT] = {
    INTR_MASK_RCNT0,
    INTR_MASK_RCNT1,
    INTR_MASK_RCNT2,
    INTR_MASK_VBLANK,
};

long SetRCnt(unsigned long spec, unsigned short target, long mode)
{
    int counter = spec & RCNT_INDEX_MASK;
    u_short hwMode = RCNT_MODE_BASE;

    if (counter >= RCNT_HW_COUNT) {
        return 0;
    }
    rcntRegs[counter].mode = 0;
    rcntRegs[counter].target = target;
    if (counter == 0 || counter == 1) {
        if (mode & RCntMdGATE) {
            hwMode |= RCNT_MODE_SYNC_ENABLE;
        }
        if (!(mode & RCntMdSC)) {
            hwMode |= RCNT_MODE_EXT_CLOCK;
        }
    } else if (counter == 2) {
        if (!(mode & RCntMdSC)) {
            hwMode |= RCNT_MODE_DIV8_CLOCK;
        }
    }
    if (mode & RCntMdINTR) {
        hwMode |= RCNT_MODE_TARGET_IRQ;
    }
    rcntRegs[counter].mode = hwMode;
    return 1;
}

long GetRCnt(unsigned long spec)
{
    int counter = spec & RCNT_INDEX_MASK;

    if (counter >= RCNT_HW_COUNT) {
        return 0;
    }
    return rcntRegs[counter].count;
}

long StartRCnt(unsigned long spec)
{
    int counter = spec & RCNT_INDEX_MASK;

    intrRegs->mask |= rcntIntrMask[counter];
    return counter < RCNT_HW_COUNT;
}

long StopRCnt(unsigned long spec)
{
    int counter = spec & RCNT_INDEX_MASK;

    intrRegs->mask &= ~rcntIntrMask[counter];
    return 1;
}

long ResetRCnt(unsigned long spec)
{
    int counter = spec & RCNT_INDEX_MASK;

    if (counter >= RCNT_HW_COUNT) {
        return 0;
    }
    rcntRegs[counter].count = 0;
    return 1;
}
