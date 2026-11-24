/* PlayStation hardware registers the Runtime Library modules use. */
#ifndef _PSX_HW_H_
#define _PSX_HW_H_

#include <sys/types.h>

#define INTR_STAT_ADDR          0x1f801070
#define INTR_MASK_ADDR          0x1f801074

/* Bus delay/size registers: the CD-ROM controller's, and the common one */
#define CDROM_DELAY_ADDR        0x1f801018
#define COM_DELAY_ADDR          0x1f801020

/* Interrupt numbers: bits of the status and mask registers. */
#define PSX_IRQ_VBLANK          0
#define PSX_IRQ_GPU             1
#define PSX_IRQ_CDROM           2
#define PSX_IRQ_DMA             3
#define PSX_IRQ_RCNT0           4
#define PSX_IRQ_RCNT1           5
#define PSX_IRQ_RCNT2           6
#define PSX_IRQ_PAD_MEMCARD     7
#define PSX_IRQ_SIO             8
#define PSX_IRQ_SPU             9
#define PSX_IRQ_PIO             10
#define PSX_IRQ_COUNT           11

#define INTR_BIT(irq)           (1 << (irq))
#define INTR_MASK_VBLANK        INTR_BIT(PSX_IRQ_VBLANK)
#define INTR_MASK_GPU           INTR_BIT(PSX_IRQ_GPU)
#define INTR_MASK_CDROM         INTR_BIT(PSX_IRQ_CDROM)
#define INTR_MASK_DMA           INTR_BIT(PSX_IRQ_DMA)
#define INTR_MASK_RCNT0         INTR_BIT(PSX_IRQ_RCNT0)
#define INTR_MASK_RCNT1         INTR_BIT(PSX_IRQ_RCNT1)
#define INTR_MASK_RCNT2         INTR_BIT(PSX_IRQ_RCNT2)
#define INTR_MASK_PAD_MEMCARD   INTR_BIT(PSX_IRQ_PAD_MEMCARD)
#define INTR_MASK_SIO           INTR_BIT(PSX_IRQ_SIO)
#define INTR_MASK_SPU           INTR_BIT(PSX_IRQ_SPU)
#define INTR_MASK_PIO           INTR_BIT(PSX_IRQ_PIO)

struct IntrReg {
    u_long stat;
    u_long mask;
};

/* The controller and memory card port (SIO0). */
#define SIO0_REG_BASE           0x1f801040

struct Sio0Reg {
    u_long data;
    u_short status;
    u_short unused0;            /* nothing reads or writes unused0 and unused1 */
    u_short mode;
    u_short control;
    u_short unused1;
    u_short baud;
};

/* DMA channels: registers MADR, BCR, CHCR at 16-byte steps from 0x1f801080. */
#define DMA_MDEC_IN             0
#define DMA_MDEC_OUT            1
#define DMA_GPU                 2
#define DMA_CDROM               3
#define DMA_SPU                 4
#define DMA_PIO                 5
#define DMA_OTC                 6
#define DMA_CH_COUNT            7

#define DMA0_MADR_ADDR          0x1f801080
#define DMA0_BCR_ADDR           0x1f801084
#define DMA0_CHCR_ADDR          0x1f801088
#define DMA1_MADR_ADDR          0x1f801090
#define DMA1_BCR_ADDR           0x1f801094
#define DMA1_CHCR_ADDR          0x1f801098
#define DMA2_MADR_ADDR          0x1f8010a0
#define DMA2_BCR_ADDR           0x1f8010a4
#define DMA2_CHCR_ADDR          0x1f8010a8
#define DMA3_MADR_ADDR          0x1f8010b0
#define DMA3_BCR_ADDR           0x1f8010b4
#define DMA3_CHCR_ADDR          0x1f8010b8
#define DMA4_MADR_ADDR          0x1f8010c0
#define DMA4_BCR_ADDR           0x1f8010c4
#define DMA4_CHCR_ADDR          0x1f8010c8
#define DMA5_MADR_ADDR          0x1f8010d0
#define DMA5_BCR_ADDR           0x1f8010d4
#define DMA5_CHCR_ADDR          0x1f8010d8
#define DMA_DPCR_ADDR           0x1f8010f0
#define DMA_DICR_ADDR           0x1f8010f4

#define DMA_BCR_COUNT_SHIFT     16
#define DMA_CHCR_BUSY_SHIFT     24
#define DMA_CHCR_BUSY           0x01000000

/* DPCR: 4 bits per channel, a priority 0..7 and bit 3 to enable. */
#define DMA_DPCR_INIT           0x33333333 /* every channel disabled, priority 3 */
#define DMA_DPCR_DISABLE_MASK   0x77777777

/* One channel's 16 bytes from DMA0_MADR_ADDR + 16 * channel. */
struct DmaChReg {
    u_long madr;
    u_long bcr;
    u_long chcr;
    u_long unused;              /* nothing reads or writes it */
};

/* DICR: bits 24..30 are the channels' interrupt flags, cleared by writing 1. */
#define DICR_BUS_ERROR          0x8000
#define DICR_IRQ_EN_SHIFT       16
#define DICR_MASTER_EN          0x800000
#define DICR_CTRL_MASK          0xffffff
#define DICR_IRQ_FLAG_SHIFT     24
#define DICR_IRQ_FLAG_MASK      0x7f
#define DICR_MASTER_FLAG        0x80000000
#define DICR_FLAG_MASK          0xff000000

/* Root counters 0..2: count, mode and target at 16-byte steps from 0x1f801100. */
#define RCNT_REG_BASE           0x1f801100
#define RCNT0_COUNT_ADDR        0x1f801100
#define RCNT0_MODE_ADDR         0x1f801104
#define RCNT0_TARGET_ADDR       0x1f801108
#define RCNT1_COUNT_ADDR        0x1f801110
#define RCNT1_MODE_ADDR         0x1f801114
#define RCNT1_TARGET_ADDR       0x1f801118
#define RCNT2_COUNT_ADDR        0x1f801120
#define RCNT2_MODE_ADDR         0x1f801124
#define RCNT2_TARGET_ADDR       0x1f801128

#define RCNT_HW_COUNT           3
#define RCNT_VBLANK             3
#define RCNT_COUNT              4
#define RCNT_INDEX_MASK         0xffff

#define RCNT_MODE_SYNC_ENABLE       0x0001
#define RCNT_MODE_SYNC_WAIT_VBLANK  0x0006 /* with SYNC_ENABLE, on counter 1:
                                              wait for the first VBLANK, then count freely */
#define RCNT_MODE_RESET_TARGET      0x0008
#define RCNT_MODE_TARGET_IRQ        0x0010
#define RCNT_MODE_OVERFLOW_IRQ      0x0020 /* at 0xffff */
#define RCNT_MODE_REPEAT_IRQ        0x0040
#define RCNT_MODE_TOGGLE_IRQ        0x0080
#define RCNT_MODE_EXT_CLOCK         0x0100 /* dot clock on counter 0, HBLANK on counter 1 */
#define RCNT_MODE_DIV8_CLOCK        0x0200 /* system clock / 8, counter 2 */

/* One counter's 16 bytes; each register is the low half of its word. */
struct RCntReg {
    u_short count;
    u_short unused0;            /* nothing reads or writes unused0..unused3 */
    u_short mode;
    u_short unused1;
    u_short target;
    u_short unused2;
    u_long unused3;
};

#define CDROM_REG0_ADDR         0x1f801800
#define CDROM_REG1_ADDR         0x1f801801
#define CDROM_REG2_ADDR         0x1f801802
#define CDROM_REG3_ADDR         0x1f801803

#define GPU_DATA_ADDR           0x1f801810
#define GPU_STATUS_ADDR         0x1f801814

/* GPU status bits */
#define GPUSTAT_VRES_480        0x00080000
#define GPUSTAT_INTERLACE_ENABLED 0x00400000
#define GPUSTAT_INTERLACE_FIELD 0x80000000

#define MDEC_DATA_ADDR          0x1f801820
#define MDEC_STAT_ADDR          0x1f801824

/* SPU registers (0x1f801c00..0x1f801dff, 16 bits each) and the SPU's bus delay register. */
#define SPU_REG_BASE            0x1f801c00
#define SPU_DELAY_ADDR          0x1f801014

/* SPU sound RAM, in bytes. */
#define SPU_RAM_SIZE            0x80000

/* Voice n's registers start at halfword n * SPU_VREG_COUNT from SPU_REG_BASE. */
#define SPU_VOICE_COUNT         24
#define SPU_VREG_COUNT          8
#define SPU_VREG_VOL_L          0
#define SPU_VREG_VOL_R          1
#define SPU_VREG_PITCH          2
#define SPU_VREG_START_ADDR     3
#define SPU_VREG_ADSR1          4
#define SPU_VREG_ADSR2          5
#define SPU_VREG_ENVX           6
#define SPU_VREG_LOOP_ADDR      7

/* Halfword offsets from SPU_REG_BASE; a _L/_H pair holds voices 0..15 and 16..23. */
#define SPU_REG_MVOL_L          0xc0
#define SPU_REG_MVOL_R          0xc1
#define SPU_REG_RVOL_L          0xc2
#define SPU_REG_RVOL_R          0xc3
#define SPU_REG_KON_L           0xc4
#define SPU_REG_KON_H           0xc5
#define SPU_REG_KOFF_L          0xc6
#define SPU_REG_KOFF_H          0xc7
#define SPU_REG_PMON_L          0xc8
#define SPU_REG_PMON_H          0xc9
#define SPU_REG_NON_L           0xca
#define SPU_REG_NON_H           0xcb
#define SPU_REG_EON_L           0xcc
#define SPU_REG_EON_H           0xcd
#define SPU_REG_MBASE           0xd1
#define SPU_REG_IRQ_ADDR        0xd2
#define SPU_REG_TSA             0xd3
#define SPU_REG_DATA            0xd4
#define SPU_REG_SPUCNT          0xd5
#define SPU_REG_TCTRL           0xd6
#define SPU_REG_SPUSTAT         0xd7
#define SPU_REG_CDVOL_L         0xd8
#define SPU_REG_CDVOL_R         0xd9
#define SPU_REG_EXTVOL_L        0xda
#define SPU_REG_EXTVOL_R        0xdb
#define SPU_REG_CUR_MVOL_L      0xdc
#define SPU_REG_CUR_MVOL_R      0xdd

#define SPUCNT_CD_MIX           0x0001
#define SPUCNT_EXT_MIX          0x0002
#define SPUCNT_CD_REVERB        0x0004
#define SPUCNT_EXT_REVERB       0x0008
#define SPUCNT_TRANS_MASK       0x0030
#define SPUCNT_TRANS_IO         0x0010
#define SPUCNT_TRANS_DMAW       0x0020
#define SPUCNT_TRANS_DMAR       0x0030
#define SPUCNT_IRQ_ENABLE       0x0040
#define SPUCNT_REVERB_SHIFT     7
#define SPUCNT_REVERB_ENABLE    0x0080
#define SPUCNT_NOISE_CLK_SHIFT  8
#define SPUCNT_NOISE_CLK_MASK   0x3f
#define SPUCNT_NOISE_CLK_BITS   (SPUCNT_NOISE_CLK_MASK << SPUCNT_NOISE_CLK_SHIFT)
#define SPUCNT_UNMUTE           0x4000
#define SPUCNT_ENABLE           0x8000
#define SPUSTAT_MASK            0x07ff
#define SPUSTAT_DMA_BUSY        0x0400
#define SPUSTAT_CAPTURE_HALF    0x0800  /* capture buffers: 1 while the second half is written */

#endif /* _PSX_HW_H_ */
