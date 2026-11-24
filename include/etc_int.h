/*
 * LIBETC's internal interface: the interrupt dispatcher's entry points and counters, shared by its
 * modules and the libraries that hook it.
 */
#ifndef _ETC_INT_H_
#define _ETC_INT_H_

#include <sys/types.h>
#include <setjmp.h>
#include <libetc.h>
#include "psx_hw.h"

/* kernel calls that kernel.h does not declare */
extern void HookEntryInt(jmp_buf env);
extern void ResetEntryInt(void);
extern void ChangeClearRCnt(long counter, long flag);

extern void (*InterruptCallback(int irq, void (*f)()))();
extern void *DMACallback(int ch, void (*f)());
extern int GetIntrMask(void);
extern int SetIntrMask(int mask);

typedef void (*IntrCallback)(void);
typedef IntrCallback (*IntrCallbackSetter)(long ch, IntrCallback func);

extern IntrCallbackSetter startIntrVSync(void);
extern IntrCallbackSetter startIntrDMA(void);
extern volatile long Vcount;
extern volatile long Hcount;

#endif /* _ETC_INT_H_ */
