/*
 * PlayStation interrupt, DMA, and VSync callback dispatcher.
 */
#include <sys/types.h>
#include <stdio.h>
#include <setjmp.h>
#include <libetc.h>
#include "psx_hw.h"
#include "etc_int.h"

/* not from kernel.h, whose _96_remove takes no argument: this module passes one */
extern void _96_remove();
extern void EnterCriticalSection(void), ExitCriticalSection(void), ReturnFromException(void);
extern void ChangeClearPAD(long clearOnVsync);

/* the dispatcher's own stack, which HookEntryInt switches to on an interrupt */
#define INTR_STACK 1024
#define INTR_STACK_MARGIN 20

#define INTR_TIMEOUT_LIMIT 2048
#define VSYNC_CALLBACK_SLOT 4 /* VSyncCallback's slot in the VSync dispatcher's table */

struct Intr {
    u_short started;                  /* the dispatcher is hooked */
    u_short inIntr;                   /* a handler is running */
    void (*handler[PSX_IRQ_COUNT])(); /* one per I_STAT bit */
    u_short mask;                     /* the bits with a handler */
    u_short savedMask;                /* I_MASK while stopped */
    u_long savedDpcr;                 /* DPCR while stopped */
    jmp_buf buf;
    int stack[INTR_STACK];
};

struct IntrControl {
    char *ver;
    IntrCallbackSetter dma;
    void (*(*setIntr)(int irq, void (*f)()))();
    void *(*start)(void);
    void *(*stop)(void);
    IntrCallbackSetter vsync;
    void *(*restart)(void);
    struct Intr *intr;
};

static void *startIntr(void);
static void trapIntr(void);
static void (*setIntr(int irq, void (*func)()))();
static void *stopIntr(void);
static void *restartIntr(void);
static void memclr(long *dst, long nwords);

/* 86 bytes; nothing reads it */
static char copyright[] =
    "Library Programs (c) 1993-1997 Sony Computer Entertainment Inc., All Rights Reserved.";
static struct Intr intr = {0, 0, {0}, 0, 0, 0};
static struct IntrControl intrControlBlock = {
    "$Id: intr.c,v 1.75 1997/02/07 09:00:36 makoto Exp $",
    0, setIntr, startIntr, stopIntr, 0, restartIntr, &intr,
};
static struct IntrControl *intrControl = &intrControlBlock;
static volatile u_short *I_STAT = (volatile u_short *)INTR_STAT_ADDR;
static volatile u_short *I_MASK = (volatile u_short *)INTR_MASK_ADDR;
static volatile u_long *DPCR = (volatile u_long *)DMA_DPCR_ADDR;
static int timeout = 0;

int ResetCallback(void)
{
    return (int)(*intrControl->start)();
}

void (*InterruptCallback(int irq, void (*f)()))()
{
    return (*intrControl->setIntr)(irq, f);
}

void *DMACallback(int ch, void (*f)())
{
    return (void *)(*intrControl->dma)(ch, f);
}

int VSyncCallback(void (*f)())
{
    return (int)(*intrControl->vsync)(VSYNC_CALLBACK_SLOT, f);
}

int VSyncCallbacks(int ch, void (*f)())
{
    return (int)(*intrControl->vsync)(ch, f);
}

int StopCallback(void)
{
    return (int)(*intrControl->stop)();
}

int RestartCallback(void)
{
    return (int)(*intrControl->restart)();
}

int CheckCallback(void)
{
    return intr.inIntr;
}

int GetIntrMask(void)
{
    return *I_MASK;
}

int SetIntrMask(int mask)
{
    int old = *I_MASK;

    *I_MASK = mask;
    return old;
}

static void *startIntr(void)
{
    if (intr.started) {
        return 0;
    }
    *I_STAT = *I_MASK = 0;
    *DPCR = DMA_DPCR_INIT;
    memclr((long *)&intr, sizeof(intr) / sizeof(long));
    if (setjmp(intr.buf)) {
        trapIntr();
    }
    intr.buf[JB_SP] = (int)&intr.stack[INTR_STACK - INTR_STACK_MARGIN];
    HookEntryInt(intr.buf);
    intr.started = 1;
    intrControl->vsync = startIntrVSync();
    intrControl->dma = startIntrDMA();
    _96_remove(intrControl);
    ExitCriticalSection();
    return &intr;
}

static void trapIntr(void)
{
    int irq;
    u_short pending;

    if (!intr.started) {
        printf("unexpected interrupt(%04x)\n", *I_STAT);
        ReturnFromException();
    }
    intr.inIntr = 1;
    while ((pending = intr.mask & *I_STAT & *I_MASK) != 0) {
        for (irq = 0; pending && irq < PSX_IRQ_COUNT; irq++, pending >>= 1) {
            if (pending & 1) {
                *I_STAT = ~INTR_BIT(irq);
                if (intr.handler[irq]) {
                    (*intr.handler[irq])();
                }
            }
        }
    }
    if (*I_STAT & *I_MASK) {
        if (timeout++ > INTR_TIMEOUT_LIMIT) {
            printf("intr timeout(%04x:%04x)\n", *I_STAT, *I_MASK);
            timeout = 0;
            *I_STAT = 0;
        }
    } else {
        timeout = 0;
    }
    intr.inIntr = 0;
    ReturnFromException();
}

static void (*setIntr(int irq, void (*func)()))()
{
    void (*old)() = intr.handler[irq];
    int mask;

    if (func != old && intr.started) {
        mask = *I_MASK;
        *I_MASK = 0;
        if (func) {
            intr.handler[irq] = func;
            mask |= INTR_BIT(irq);
            intr.mask |= INTR_BIT(irq);
        } else {
            intr.handler[irq] = 0;
            mask &= ~INTR_BIT(irq);
            intr.mask &= ~INTR_BIT(irq);
        }
        if (irq == PSX_IRQ_VBLANK) {
            ChangeClearPAD(func == 0);
            ChangeClearRCnt(RCNT_VBLANK, func == 0);
        }
        if (irq == PSX_IRQ_RCNT0) {
            ChangeClearRCnt(0, func == 0);
        }
        if (irq == PSX_IRQ_RCNT1) {
            ChangeClearRCnt(1, func == 0);
        }
        if (irq == PSX_IRQ_RCNT2) {
            ChangeClearRCnt(2, func == 0);
        }
        *I_MASK = mask;
    }
    return old;
}

static void *stopIntr(void)
{
    if (!intr.started) {
        return 0;
    }
    EnterCriticalSection();
    intr.savedMask = *I_MASK;
    intr.savedDpcr = *DPCR;
    *I_STAT = *I_MASK = 0;
    *DPCR &= DMA_DPCR_DISABLE_MASK;
    ResetEntryInt();
    intr.started = 0;
    return &intr;
}

static void *restartIntr(void)
{
    if (intr.started) {
        return 0;
    }
    HookEntryInt(intr.buf);
    intr.started = 1;
    *I_MASK = intr.savedMask;
    *DPCR = intr.savedDpcr;
    ExitCriticalSection();
    return &intr;
}

static void memclr(long *dst, long nwords)
{
    while (nwords--) {
        *dst++ = 0;
    }
}
