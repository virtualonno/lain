/*
 * CD-ROM streaming interrupt handler (StCdInterrupt) and DMA transfer helpers.
 */
#include <sys/types.h>
#include <stdio.h>
#include <libcd.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

static volatile u_long *CDROM_DELAY = (volatile u_long *)CDROM_DELAY_ADDR;
static volatile u_long *COM_DELAY = (volatile u_long *)COM_DELAY_ADDR;
static volatile u_long *DMA_DPCR = (volatile u_long *)DMA_DPCR_ADDR;
static volatile u_long *DICR = (volatile u_long *)DMA_DICR_ADDR;
static volatile u_long *DMA1_CHCR = (volatile u_long *)DMA1_CHCR_ADDR;
/* of the nine pointers from here only DMA3_CHCR is read: nothing reads the other eight (32 bytes) */
static volatile u_long *DMA1_MADR = (volatile u_long *)DMA1_MADR_ADDR;
static volatile u_long *DMA2_CHCR = (volatile u_long *)DMA2_CHCR_ADDR;
static volatile u_long *DMA2_MADR = (volatile u_long *)DMA2_MADR_ADDR;
static volatile u_long *DMA3_CHCR = (volatile u_long *)DMA3_CHCR_ADDR;
static volatile u_long *DMA3_MADR = (volatile u_long *)DMA3_MADR_ADDR;
static volatile u_long *DMA5_CHCR = (volatile u_long *)DMA5_CHCR_ADDR;
static volatile u_long *DMA5_MADR = (volatile u_long *)DMA5_MADR_ADDR;
static volatile u_long *INTR_STAT = (volatile u_long *)INTR_STAT_ADDR;
static volatile u_long *INTR_MASK = (volatile u_long *)INTR_MASK_ADDR;

u_long debug_cause = 0;
/* 4 bytes; nothing reads it */
static u_long unused_word = 0;

static void mem2mem(u_long *dst, u_long *src, u_long size, u_long flag);
static void dma_execute(int ch, u_long addr, u_long bcount, u_long bsize, u_long mode, u_char intr, u_long unread);

void StCdInterrupt(void)
{
	static volatile u_short *HeaderArea;
	volatile u_short status[4];
	CdlLOC loc;
	u_char result[CD_RESULT_SIZE];
	u_long i;
	u_long dma_mode;

	if (StFinalSector == 1)
		return;

	if (StRgb24 && (*DMA1_CHCR & DMA_CHCR_BUSY)) {
		StCdIntrFlag = 1;
		if (StEmu_Addr)
			StEmu_Idx++;
		debug_cause = 1;
		return;
	}

	if (CdReady(1, result) == CdlDiskError)
		return;

	status[1] = result[0];
	status[2] = result[1];
	if (status[1] & CdlStatSeekError) {
		debug_cause = 3;
		return;
	}

	HeaderArea = (volatile u_short *)(StRingAddr + StRingIdx1 * HEADER_SIZE);
	if (HeaderArea[StSTATUS] != StFREE) {
		if (StEmu_Addr)
			StEmu_Idx++;
		debug_cause = 4;
		return;
	}

	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = CD_REQ_CLEAR;
	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = CD_REQ_BFRD;
	*CDROM_DELAY = CDROM_DELAY_DMA;
	*COM_DELAY = COM_DELAY_DMA;

	/* In 2340-byte mode, read the 4-byte Mode 2 header (CdlLOC) and skip the 8-byte XA subheader */
	if (StMode == ST_MODE_2340) {
		for (i = 0; i < sizeof(CdlLOC); i++)
			((u_char *)&loc)[i] = *CD_REG2;
		for (i = 0; i < 8; i++)
			*CD_REG2;
	}

	dma_mode = DMA_CHCR_CDROM_READ;
	if (StEmu_Addr) {
		mem2mem((u_long *)HeaderArea, StEmu_Addr + StEmu_Idx * SECTOR_SIZE, HEADER_SIZE, 0);
	} else {
		dma_execute(DMA_CDROM, (u_long)HeaderArea, 0, HEADER_SIZE, dma_mode, 0, 0);
	}

	while (*DMA3_CHCR & DMA_CHCR_BUSY)
		;

	((StHEADER *)HeaderArea)->loc = loc;
	*CDROM_DELAY = CDROM_DELAY_DEFAULT;
	*COM_DELAY = COM_DELAY_DEFAULT;

	if (StSTART_FLAG == 1) {
		if (StStartFrame != 0) {
			if (StStartFrame != HeaderArea[StFRAME_NO]) {
				HeaderArea[StSTATUS] = StFREE;
				if (StEmu_Addr)
					StEmu_Idx++;
				return;
			}
			StSTART_FLAG = 0;
		}
	}

	if (HeaderArea[StVER] != ST_HEADER_VERSION ||
	    ((HeaderArea[StTYPE] >> ST_TYPE_CHANNEL_SHIFT) & ST_TYPE_CHANNEL_MASK) != CChannel) {
		if (StEmu_Addr)
			StEmu_Idx = 0;
		else
			HeaderArea[StVER];
		debug_cause = 5;
		HeaderArea[StSTATUS] = StFREE;
		return;
	}

	if (Stsector_offset != HeaderArea[StSECTOR_OFFSET] ||
	    (Stframe_no != 0 && Stframe_no != HeaderArea[StFRAME_NO])) {
		Stframe_no = 0;
		Stsector_offset = 0;
		init_ring_status(StRingIdx2, StRingIdx1 - StRingIdx2);
		StRingIdx1 = StRingIdx2;
		HeaderArea[StSTATUS] = StFREE;
		if (StEmu_Addr)
			StEmu_Idx++;
		debug_cause = 6;
		return;
	}

	if (HeaderArea[StSECTOR_OFFSET] == 0) {
		Stsector_offset = 0;
		Stframe_no = HeaderArea[StFRAME_NO];
		if (StEndFrame != 0 && Stframe_no >= StEndFrame) {
			Stframe_no = 0;
			Stsector_offset = 0;
			init_ring_status(StRingIdx2, StRingIdx1 - StRingIdx2);
			StRingIdx1 = StRingIdx2;
			HeaderArea[StSTATUS] = StFREE;
			StSTART_FLAG = 1;
			if (StFunc2)
				(*StFunc2)();
			if (StEmu_Addr)
				StEmu_Idx++;
			debug_cause = 7;
			return;
		}
		if (StRingSize - StRingIdx1 - 1 < HeaderArea[StSECTOR_SIZE]) {
			if (StEndFrame == 0) {
				HeaderArea[StSTATUS] = StREWIND;
				StSTART_FLAG = 1;
				if (StFunc2)
					(*StFunc2)();
				if (StEmu_Addr)
					StEmu_Idx++;
				debug_cause = 8;
				return;
			}
			if (((short *)StRingAddr)[StSTATUS] != StFREE) {
				HeaderArea[StSTATUS] = StFREE;
				if (StEmu_Addr)
					StEmu_Idx++;
				debug_cause = 9;
				return;
			}
			HeaderArea[StSTATUS] = StREWIND;
			StRingIdx1 = 0;
			for (i = 0; i < HEADER_SIZE; i++)
				StRingAddr[i] = ((u_long *)HeaderArea)[i];
			HeaderArea = (volatile u_short *)StRingAddr;
		}
		StRingIdx2 = StRingIdx1;
	}

	debug_cause = 10;
	Stsector_offset++;
	StRingBase = StRingAddr + StRingSize * HEADER_SIZE + StRingIdx1 * (SECTOR_SIZE - HEADER_SIZE);

	if (StRgb24) {
		dma_mode = DMA_CHCR_CDROM_READ;
		*CDROM_DELAY = CDROM_DELAY_DMA;
		*COM_DELAY = COM_DELAY_DMA;
	} else {
		*CDROM_DELAY = CDROM_DELAY_CHOP;
		dma_mode = DMA_CHCR_CDROM_CHOP;
	}

	if (HeaderArea[StSECTOR_SIZE] - 1 == HeaderArea[StSECTOR_OFFSET]) {
		StFinalSector = 1;
		if (StEmu_Addr) {
			mem2mem(StRingBase, StEmu_Addr + StEmu_Idx * SECTOR_SIZE + HEADER_SIZE, SECTOR_SIZE - HEADER_SIZE, 1);
			StEmu_Idx++;
		} else {
			dma_execute(DMA_CDROM, (u_long)StRingBase, 0, SECTOR_SIZE - HEADER_SIZE, dma_mode, 1, 0);
		}
		Stsector_offset = 0;
		Stframe_no = 0;
		CChannel = StCHANNEL;
	} else {
		if (StEmu_Addr) {
			mem2mem(StRingBase, StEmu_Addr + StEmu_Idx * SECTOR_SIZE + HEADER_SIZE, SECTOR_SIZE - HEADER_SIZE, 0);
			StEmu_Idx++;
		} else {
			dma_execute(DMA_CDROM, (u_long)StRingBase, 0, SECTOR_SIZE - HEADER_SIZE, dma_mode, 0, 0);
		}
	}

	*COM_DELAY = COM_DELAY_DEFAULT;
	HeaderArea[StSTATUS] = StBUSY;
	StRingIdx1++;
	if (StEmu_Addr && StFinalSector) {
		data_ready_callback();
	}
}

static void mem2mem(u_long *dst, u_long *src, u_long size, u_long flag)
{
	u_long i;

	for (i = 0; i < size; i++) {
		*dst++ = *src++;
	}
}

static void dma_execute(int ch, u_long addr, u_long bcount, u_long bsize, u_long mode, u_char intr, u_long unread)
{
	volatile u_long readback;
	volatile u_long *dma_reg;
	int wait_count;
	u_long dpcr_mask;
	u_long dpcr_val;
	u_char irq_mask;

	wait_count = 0;
	while (*(volatile u_long *)(DMA0_CHCR_ADDR + (ch << 4)) & DMA_CHCR_BUSY) {
		if (wait_count == DMA_WAIT_TIMEOUT) {
			printf("DMA STATUS ERROR %x\n", *(volatile u_long *)(DMA0_CHCR_ADDR + (ch << 4)));
			break;
		}
		wait_count++;
	}

	if (intr == 1) {
		irq_mask = ((u_char *)DICR)[2];
		irq_mask |= 1 << ch;
		((u_char *)DICR)[2] = irq_mask;
		readback = *DICR;
	} else {
		irq_mask = ((u_char *)DICR)[2];
		irq_mask &= ~(1 << ch);
		((u_char *)DICR)[2] = irq_mask;
		readback = *DICR;
	}

	/* Enable DMA channel ch in DPCR (bit ch * 4 + 3) */
	dpcr_val = ch << 2;
	dpcr_val += 3;
	dpcr_mask = 1;
	dpcr_mask <<= dpcr_val;
	dma_reg = (volatile u_long *)(DMA0_MADR_ADDR + (ch << 4));
	dpcr_val = *DMA_DPCR;
	dpcr_val |= dpcr_mask;
	*DMA_DPCR = dpcr_val;
	*dma_reg++ = addr;
	*dma_reg++ = (bcount << 16) | bsize;
	while ((*CD_REG0 & CD_STAT_DRQSTS) == 0)
		;
	*dma_reg = mode;
	readback = *dma_reg;
}
