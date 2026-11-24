/*
 * Internal definitions and interfaces for LIBSPU.
 */
#ifndef _SPU_INT_H_
#define _SPU_INT_H_

#include <sys/types.h>
#include <libspu.h>
#include "psx_hw.h"
#include "spu_malloc.h"
#include "spu_reverb.h"

/* _SpuInit() / _spu_init() mode: cold init resets voices and centerNote. */
#define SPU_INIT_COLD           0
#define SPU_INIT_HOT            1

#define SPU_RQ_COUNT            10      /* mirrors KON..EON (0xc4..0xcd) */

#define SPU_TCTRL_NORMAL        0x0004
#define SPU_WAIT_TIMEOUT        0x0f00

/* Volume register (MVOL / voice VOL) mode bits and value limits:
 * bit 15 = 0: direct 15-bit signed volume;
 * bit 15 = 1: sweep (bits 14..12 = curve/direction/phase, bits 6..0 = rate). */
#define SPU_VOL_DIRECT_MASK         0x7fff
#define SPU_VOL_SWEEP_RATE_MAX      0x7f
#define SPU_VOL_MODE_DIRECT         0x0000
#define SPU_VOL_SWEEP_LINEAR_INC_N  0x8000
#define SPU_VOL_SWEEP_LINEAR_INC_R  0x9000
#define SPU_VOL_SWEEP_LINEAR_DEC_N  0xa000
#define SPU_VOL_SWEEP_LINEAR_DEC_R  0xb000
#define SPU_VOL_SWEEP_EXP_INC_N     0xc000
#define SPU_VOL_SWEEP_EXP_INC_R     0xd000
#define SPU_VOL_SWEEP_EXP_DEC       0xe000

/* _spu_t() transfer sub-commands and return status. */
#define SPU_TRANS_READ          0
#define SPU_TRANS_WRITE         1
#define SPU_TRANS_ADDR          2
#define SPU_TRANS_START         3
#define SPU_TRANS_TIMEOUT       (-2)

/* _spu_FsetRXX() mode: write raw 16-bit value vs. scaled byte address. */
#define SPU_FSETRXX_RAW         0

/* Sound RAM extent in scaled units (512 KB = 0x10000 << _spu_mem_mode_plus) and zero buffer size. */
#define SPU_RAM_UNITS           0x10000
#define SPU_ZEROBUF_SIZE        0x400

extern volatile unsigned short *_spu_RXX;
extern unsigned short _spu_tsa;
extern long _spu_transMode;
extern long _spu_trans_mode;
extern long _spu_addrMode;
extern long _spu_mem_mode;
extern long _spu_mem_mode_plus;
extern long _spu_mem_mode_unit;
extern long _spu_mem_mode_unitM;
extern long _spu_inTransfer;          /* 0 while a transfer runs, 1 when none does */
extern volatile SpuTransferCallbackProc _spu_transferCallback;
extern volatile SpuIRQCallbackProc _spu_IRQCallback;
extern volatile unsigned short _spu_RQ[SPU_RQ_COUNT];

extern long _spu_EVdma;
extern unsigned long _spu_keystat;
extern long _spu_rev_flag;
extern long _spu_rev_reserve_wa;
extern long _spu_rev_offsetaddr;
extern SpuReverbAttr _spu_rev_attr;
extern volatile unsigned long _spu_RQvoice;
extern volatile unsigned long _spu_RQmask;
extern unsigned short _spu_voice_centerNote[SPU_VOICE_COUNT];
extern unsigned char _spu_zerobuf[SPU_ZEROBUF_SIZE];
extern unsigned long _spu_env;
extern long _spu_isCalled;

extern long _spu_rev_startaddr[SPU_REV_MODE_MAX];
extern SpuReverbPreset _spu_rev_param[SPU_REV_MODE_MAX];

extern SpuMemBlock *_spu_memList;
extern long _spu_AllocBlockNum;
extern long _spu_AllocLastNum;

extern long _spu_init(long mode);
extern void _SpuInit(long mode);
extern void _spu_FiDMA(void);
extern void _spu_Fr_(unsigned long addr, unsigned short tsa, unsigned long blocks);
extern long _spu_t(long mode, ...);
extern unsigned long _spu_Fw(unsigned char *addr, unsigned long size);
extern void _spu_FsetRXX(long reg, unsigned long value, long mode);
extern unsigned long _spu_FsetRXXa(long reg, unsigned long value);
extern unsigned long _spu_FgetRXXa(long reg, long mode);
extern void _spu_FsetPCR(long high_priority);
extern void _SpuDataCallback(void (*func)(void));
extern unsigned long _SpuSetAnyVoice(long on_off, unsigned long voice_bit, long reg_lo, long reg_hi);
extern long _SpuIsInAllocateArea(unsigned long addr);
extern long _SpuIsInAllocateArea_(unsigned long addr);
extern void _spu_gcSPU(void);
extern void _spu_setInTransfer(int flag);
extern int _spu_getInTransfer(void);
extern int _spu_note2pitch(short center_note, short center_fraction,
                           short note, short fraction);
extern unsigned long _spu_pitch2note(int center_note, int center_fraction,
                                     unsigned short pitch);

#endif /* _SPU_INT_H_ */
