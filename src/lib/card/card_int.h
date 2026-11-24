/*
 * Internal kernel stub and patch interfaces for LIBCARD.
 */
#ifndef _CARD_INT_H_
#define _CARD_INT_H_

extern void InitCARD2(long mode);
extern long StartCARD2(void);
extern long StopCARD2(void);
extern void _patch_card(void);
extern void _patch_card2(void);
extern void _ExitCard(void);
extern void _copy_memcard_patch(void);

#endif
