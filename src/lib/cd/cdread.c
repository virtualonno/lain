/* CD-ROM sector read in the background: CdRead starts it, the ready interrupt
 * stores each sector and restarts the read on an error, CdReadSync waits for
 * or polls it and restarts a stalled read. */

#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include <libcd.h>

/* Sector transfer lengths in 32-bit words */
#define SECTOR_HEAD_WORDS	(12 / 4)	/* 4-byte header + 8-byte XA subheader */
#define SECTOR_2048_WORDS	(2048 / 4)	/* 2048-byte user data sector */
#define SECTOR_2328_WORDS	(2328 / 4)	/* 2328-byte sector (CdlModeSize0) */
#define SECTOR_2340_WORDS	(2340 / 4)	/* 2340-byte full sector (CdlModeSize1) */

/* in vertical blanks */
#define READ_TIMEOUT_VSYNC	(20 * 60)
#define RETRY_TIMEOUT_VSYNC	60

extern CdlCB CD_cbread;
extern int CD_read_dma_mode;

#define READ_CHOPPED_DMA 1

/* Active read state shared with the data-ready interrupt callback */
static volatile struct {
	int nsector;		/* sectors requested */
	u_long *buf;		/* caller's destination buffer */
	u_long *addr;		/* write pointer for the next sector */
	int mode;		/* CD-ROM read mode */
	int ssize;		/* words per sector */
	int rsector;		/* sectors remaining; -1 on error */
	int rtime;		/* VSync count at the last ready interrupt or read start */
	int stime;		/* VSync count at CdRead, reset while the shell is open */
	int pos;		/* expected logical sector number of next sector */
	CdlCB oldsync;		/* saved CdSyncCallback */
	CdlCB oldready;		/* saved CdReadyCallback */
	void (*olddata)(void);	/* saved CdDataCallback */
	u_char *result;		/* status buffer from the ready interrupt */
} rd = {0};

static void cb_read_dma(void);
static int cd_read_start(int retry);

static void cb_read(u_char intr, u_char *result)
{
	u_long head[SECTOR_HEAD_WORDS];

	rd.result = result;
	if (intr == CdlDataReady) {
		if (rd.rsector > 0) {
			if (rd.ssize == SECTOR_2048_WORDS) {
				if (CD_read_dma_mode & READ_CHOPPED_DMA) {
					CdDataCallback(0);
					CdGetSector2(head, SECTOR_HEAD_WORDS);
					CdDataSync(0);
					CdDataCallback(cb_read_dma);
				} else
					CdGetSector(head, SECTOR_HEAD_WORDS);
				if (CdPosToInt((CdlLOC *)head) != rd.pos) {
					puts("CdRead: sector error\n");
					rd.rsector = -1;
				}
			}
			if (CD_read_dma_mode & READ_CHOPPED_DMA)
				CdGetSector2(rd.addr, rd.ssize);
			else {
				CdGetSector(rd.addr, rd.ssize);
				rd.addr += rd.ssize;
				rd.rsector--;
				rd.pos++;
			}
		}
	} else
		rd.rsector = -1;

	rd.rtime = VSync(-1);
	if (rd.rsector < 0)
		cd_read_start(1);
	if (VSync(-1) > rd.stime + READ_TIMEOUT_VSYNC)
		rd.rsector = -1;
	if (rd.rsector == 0 || VSync(-1) > rd.stime + READ_TIMEOUT_VSYNC) {
		CdReadyCallback(rd.oldready);
		if (CD_read_dma_mode & READ_CHOPPED_DMA)
			CdDataCallback(rd.olddata);
		CdControlB(CdlPause, 0, 0);
		CdSyncCallback(rd.oldsync);
		if (CD_cbread)
			(*CD_cbread)(rd.rsector == 0 ? CdlComplete : CdlDiskError, result);
	}
}

static void cb_read_dma(void)
{
	rd.addr += rd.ssize;
	rd.rsector--;
	rd.pos++;
	if (rd.rsector == 0) {
		CdReadyCallback(rd.oldready);
		if (CD_read_dma_mode & READ_CHOPPED_DMA)
			CdDataCallback(rd.olddata);
		CdControlB(CdlPause, 0, 0);
		CdSyncCallback(rd.oldsync);
		if (CD_cbread)
			(*CD_cbread)(CdlComplete, rd.result);
	}
}

static int cd_read_start(int retry)
{
	u_char param[4];

	CdSyncCallback(0);
	CdReadyCallback(0);
	if (CD_read_dma_mode & READ_CHOPPED_DMA)
		CdDataCallback(0);
	if (CdStatus() & CdlStatShellOpen) {
		if ((VSync(-1) & 0x3f) == 0)
			puts("CdRead: Shell open...\n");
		CdControlF(CdlNop, 0);
		rd.stime = VSync(-1);
		return rd.rsector = -1;
	}
	if (retry) {
		puts("CdRead: retry...\n");
		CdControl(CdlPause, 0, 0);
		if (CdControl(CdlSetloc, (u_char *)CdLastPos(), 0) == 0) {
			return rd.rsector = -1;
		}
	}
	CdFlush();
	if ((param[0] = rd.mode) != CdMode() || retry) {
		if (CdControl(CdlSetmode, param, 0) == 0) {
			return rd.rsector = -1;
		}
	}
	rd.pos = CdPosToInt(CdLastPos());
	CdReadyCallback(cb_read);
	if (CD_read_dma_mode & READ_CHOPPED_DMA)
		CdDataCallback(cb_read_dma);
	rd.addr = rd.buf;
	CdControlF(CdlReadN, 0);
	rd.rsector = rd.nsector;
	rd.rtime = VSync(-1);
	return rd.rsector;
}

void CdReadBreak(void)
{
	if (CD_read_dma_mode & READ_CHOPPED_DMA)
		CdDataSync(0);
	rd.rsector = 0;
	CdReadyCallback(rd.oldready);
	if (CD_read_dma_mode & READ_CHOPPED_DMA)
		CdDataCallback(rd.olddata);
	CdControlB(CdlPause, 0, 0);
	CdSyncCallback(rd.oldsync);
}

int CdRead(int sectors, u_long *buf, int mode)
{
	rd.mode = mode;
	switch (rd.mode & (CdlModeSize0 | CdlModeSize1)) {
	case 0:
		rd.ssize = SECTOR_2048_WORDS;
		break;
	case CdlModeSize1:
		rd.ssize = SECTOR_2340_WORDS;
		break;
	default:
		rd.ssize = SECTOR_2328_WORDS;
		break;
	}
	rd.mode |= CdlModeSize1;
	rd.buf = buf;
	rd.nsector = sectors;
	rd.oldsync = CdSyncCallback(0);
	rd.oldready = CdReadyCallback(0);
	if (CD_read_dma_mode & READ_CHOPPED_DMA)
		rd.olddata = CdDataCallback(0);
	rd.stime = VSync(-1);
	if (CdStatus() & (CdlStatSeek | CdlStatRead | CdlStatPlay))
		CdControlB(CdlPause, 0, 0);
	return cd_read_start(0) > 0;
}

int CdReadSync(int mode, u_char *result)
{
	int ret;

	do {
		if (VSync(-1) > rd.stime + READ_TIMEOUT_VSYNC)
			ret = -1;
		else if (rd.rsector < 0 || VSync(-1) > rd.rtime + RETRY_TIMEOUT_VSYNC) {
			cd_read_start(1);
			ret = rd.nsector;
		} else
			ret = rd.rsector;
	} while (mode == 0 && ret > 0);
	CdReady(1, result);
	return ret;
}

CdlCB CdReadCallback(CdlCB func)
{
	CdlCB old = CD_cbread;

	CD_cbread = func;
	return old;
}

int CdReadMode(int mode)
{
	int old = CD_read_dma_mode;

	CD_read_dma_mode = mode;
	return old;
}
