/*
 * Internal hardware definitions and interfaces for LIBGPU.
 */
#ifndef _GPU_INT_H_
#define _GPU_INT_H_

#include <sys/types.h>
#include "psx_hw.h"

extern void GPU_cw(u_long addr);

#define DMA6_MADR_ADDR		0x1f8010e0
#define DMA6_BCR_ADDR		0x1f8010e4
#define DMA6_CHCR_ADDR		0x1f8010e8

#define DMA_DPCR_GPU_EN		0x00000800	/* DMA2 (GPU) enable */
#define DMA_DPCR_OTC_EN		0x08000000	/* DMA6 (ordering table clear) enable */
#define DMA_CHCR_GPU_IDLE	0x00000401	/* RAM -> GPU, linked-list mode, stopped */
#define DMA_CHCR_GPU_LIST	0x01000401	/* RAM -> GPU, linked-list mode, start */
#define DMA_CHCR_GPU_WRITE	0x01000201	/* RAM -> GPU, block sync mode, start */
#define DMA_CHCR_GPU_READ	0x01000200	/* GPU -> RAM, block sync mode, start */
#define DMA_CHCR_OTC_CLEAR	0x11000002	/* reverse address step, burst, start */
#define GPU_DMA_BLOCK_WORDS	0x10

/* DMA packet tags: length in the top byte, next packet's address below */
#define GPU_ADDR_MASK		0x00ffffff
#define GPU_DMA_END		0x00ffffff
#define GPU_COLOR_MASK		0x00ffffff
#define GPU_INFO_MASK		0x00ffffff
#define GPU_DMA_TAG(len, addr)	(((u_long)(len) << 24) | ((u_long)(addr) & GPU_ADDR_MASK))

#define GP0_CLEAR_CACHE		0x01000000
#define GP0_FILL_RECT		0x02000000
#define GP0_TILE		0x60000000	/* monochrome rectangle of any size */
#define GP0_VRAM_BLIT		0x80000000	/* VRAM -> VRAM copy */
#define GP0_CPU_TO_VRAM		0xa0000000
#define GP0_CPU_TO_VRAM_ALT	0xb0000000	/* also a CPU -> VRAM copy (0xa0..0xbf) */
#define GP0_VRAM_TO_CPU		0xc0000000
#define GP0_DRAW_MODE		0xe1000000
#define GP0_TEX_WINDOW		0xe2000000
#define GP0_DRAW_AREA_TL	0xe3000000
#define GP0_DRAW_AREA_BR	0xe4000000
#define GP0_DRAW_OFFSET		0xe5000000
#define GP0_MASK_BIT		0xe6000000

/* GP0_DRAW_MODE bits; _version sets GP0_MODE_2M_DFE on a GPU whose type does not read 2 */
#define GP0_MODE_DTD		0x0200		/* dither */
#define GP0_MODE_DFE		0x0400		/* draw into the displayed area */
#define GP0_MODE_TPAGE_MASK	0x09ff
#define GP0_MODE_2M_DFE		0x1000

#define GP0_MASK_SET		0x01		/* set the mask bit while drawing */
#define GP0_MASK_CHECK		0x02		/* skip pixels whose mask bit is set */

#define GP1_RESET		0x00000000
#define GP1_RESET_FIFO		0x01000000
#define GP1_ACK_IRQ		0x02000000
#define GP1_DISP_ON		0x03000000
#define GP1_DISP_OFF		0x03000001
#define GP1_DMA_OFF		0x04000000
#define GP1_DMA_CPU_TO_GP0	0x04000002
#define GP1_DMA_GPU_TO_CPU	0x04000003
#define GP1_DISP_START		0x05000000
#define GP1_DISP_HRANGE		0x06000000
#define GP1_DISP_VRANGE		0x07000000
#define GP1_DISP_MODE		0x08000000
#define GP1_EXT_VRAM		0x09000001	/* 2 MB of video memory, on a GPU whose type reads 2 */
#define GP1_GET_INFO		0x10000000

/* GP1_DISP_MODE bits; GP1_CTL_DISP_MODE indexes ctlbuf */
#define GP1_CTL_DISP_MODE	8
#define GP1_DMODE_HRES_320	0x01
#define GP1_DMODE_HRES_512	0x02
#define GP1_DMODE_HRES_640	0x03
#define GP1_DMODE_VRES_480	0x04
#define GP1_DMODE_PAL		0x08
#define GP1_DMODE_RGB24		0x10
#define GP1_DMODE_INTERLACE	0x20
#define GP1_DMODE_HRES_368	0x40
#define GP1_DMODE_REVERSE	0x80

/* GP1_GET_INFO sub-codes */
#define GP1_INFO_DRAW_AREA_TL	3
#define GP1_INFO_DRAW_AREA_BR	4
#define GP1_INFO_DRAW_OFFSET	5
#define GP1_INFO_GPU_TYPE	7

#define GPUSTAT_DRAW_MODE_MASK	0x000007ff
#define GPUSTAT_2M_DRAW_MODE_MASK 0x00003fff
#define GPUSTAT_CMD_READY	0x04000000	/* ready for a GP0 command word */
#define GPUSTAT_VRAM_READY	0x08000000	/* ready to send VRAM to the CPU */

/* ClearImage2 packs its DFE flag in bit 31 of the colour */
#define CLR_FLAG_DFE		0x80000000

#define GPU_QUE_SIZE		64
#define GPU_QUE_MASK		(GPU_QUE_SIZE - 1)
#define GPU_ALARM_VSYNC		240
#define GPU_ALARM_LIMIT		0xf0000

#endif /* _GPU_INT_H_ */
