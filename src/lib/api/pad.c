/* The controller driver's start and stop, and the interrupt entry that clears the controller
 * port's control register on each vertical blank. */
#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include "api_int.h"
#include "psx_hw.h"

#define PAD_RESET_WAIT 10

static long InitPadFlag = 0;
static volatile struct Sio0Reg *volatile pad = (struct Sio0Reg *)SIO0_REG_BASE;
static volatile struct IntrReg *volatile intr = (struct IntrReg *)INTR_STAT_ADDR;

static struct IntrChainEntry padIntrEntry;

static long SetPatchPad(void);
static long RemovePatchPad(void);

void SetInitPadFlag(long flag)
{
    InitPadFlag = flag;
}

long ReadInitPadFlag(void)
{
    return InitPadFlag;
}

/* ignoredA and ignoredB are forwarded unchanged to PAD_init2 (BIOS B0:15); PadInit leaves them unset. */
long PAD_init(long mode, long *tbl, long ignoredA, long ignoredB)
{
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    PAD_init2(mode, tbl, ignoredA, ignoredB);
    InitPadFlag = 1;
    return 1;
}

long InitPAD(char *bufA, long lenA, char *bufB, long lenB)
{
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    InitPAD2(bufA, lenA, bufB, lenB);
    InitPadFlag = 1;
    return 1;
}

long StartPAD(void)
{
    StartPAD2();
    ChangeClearPAD(0);
    EnablePAD();
    return 1;
}

void StopPAD(void)
{
    DisablePAD();
    StopPAD2();
    RemovePatchPad();
    InitPadFlag = 0;
}

static long ClearPadControl(void);
static long IsVBlankPending(void);

static inline void setup_patch_rp(long (*handler)(void), long (*check)(void))
{
    padIntrEntry.handler = handler;
    padIntrEntry.check = check;
    padIntrEntry.next = 0;
    padIntrEntry.unused = 0;
}

static long SetPatchPad(void)
{
    EnterCriticalSection();
    setup_patch_rp(ClearPadControl, IsVBlankPending);
    SysDeqIntRP(PAD_INTR_PRIORITY, &padIntrEntry);
    SysEnqIntRP(PAD_INTR_PRIORITY, &padIntrEntry);
    ExitCriticalSection();
    return 1;
}

static long RemovePatchPad(void)
{
    EnterCriticalSection();
    SysDeqIntRP(PAD_INTR_PRIORITY, &padIntrEntry);
    ExitCriticalSection();
    return 1;
}

static inline long wait_for_pad_reset(volatile long count)
{
    --count;
    if (count == -1)
        return 0;
    do {
        --count;
    } while (count != -1);
    return 0;
}

static long ClearPadControl(void)
{
    pad->control = 0;
    return wait_for_pad_reset(PAD_RESET_WAIT);
}

static long IsVBlankPending(void)
{
    struct IntrReg *regs = intr;

    if (!(regs->mask & INTR_MASK_VBLANK) || !(regs->stat & INTR_MASK_VBLANK)) {
        return 0;
    }
    return 1;
}
