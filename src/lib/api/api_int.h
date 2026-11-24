/*
 * Internal kernel stub and patch interfaces for LIBAPI.
 */
#ifndef _API_INT_H_
#define _API_INT_H_

/* An entry of the kernel's interrupt chains, as SysEnqIntRP and SysDeqIntRP take it. */
struct IntrChainEntry {
    struct IntrChainEntry *next;
    long (*handler)(void);
    long (*check)(void);
    long unused; /* zeroed by setup_patch_rp; nothing in the program reads it */
};

#define PAD_INTR_PRIORITY 1

extern long ReadInitPadFlag(void);

extern void _patch_pad(void);
extern void _remove_ChgclrPAD(void);
extern long PAD_init2(long mode, long *tbl, long ignoredA, long ignoredB);
extern long InitPAD2(char *bufA, long lenA, char *bufB, long lenB);
extern long StartPAD2(void);
extern void StopPAD2(void);
extern void SysEnqIntRP(int priority, void *chain);
extern void SysDeqIntRP(int priority, void *chain);

#endif /* _API_INT_H_ */
