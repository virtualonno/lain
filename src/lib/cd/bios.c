/*
 * Low-level CD-ROM controller BIOS: commands, interrupts, volume, and DMA.
 */
#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include <libcd.h>
#include "cd_int.h"
#include "etc_int.h"

/* Bitmask returned by getintr() to tell callback() which handlers to invoke */
#define INTR_FLAG_ACK		1	/* Acknowledge received; Complete still pending */
#define INTR_FLAG_SYNC		2	/* command finished; invoke CD_cbsync */
#define INTR_FLAG_READY		4	/* data ready/end; invoke CD_cbready */

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static u_long cd_id[2] = {0x25047350, 0x0043529b};

CdlCB CD_cbsync = 0;
CdlCB CD_cbready = 0;
int CD_debug = 0;
int CD_status = 0;
int CD_status1 = 0;
int CD_nopen = 0;
CdlLOC CD_pos = {2, 0, 0, 0};
u_char CD_mode = 0;
u_char CD_com = 0;
int DS_active = 0;
u_long StMode;

char *CD_comstr[CD_COM_MAX] = {
	"CdlSync", "CdlNop", "CdlSetloc", "CdlPlay",
	"CdlForward", "CdlBackward", "CdlReadN", "CdlStandby",
	"CdlStop", "CdlPause", "CdlReset", "CdlMute",
	"CdlDemute", "CdlSetfilter", "CdlSetmode", "?",
	"CdlGetlocL", "CdlGetlocP", "?", "CdlGetTN",
	"CdlGetTD", "CdlSeekL", "CdlSeekP", "?",
	"?", "?", "?", "CdlReadS",
	"?", "?", "?", "?",
};

char *CD_intstr[CD_INT_MAX] = {
	"NoIntr", "DataReady", "Complete", "Acknowledge",
	"DataEnd", "DiskError", "?", "?",
};

/* Per-command dispatch properties, indexed by command number */
static struct {
	int sync[CD_COM_MAX];	/* answers with a second interrupt (Complete) after its Acknowledge */
	int ready[CD_COM_MAX];	/* CD_cw clears Intr[1] (DataReady, DataEnd) before sending it */
	int stat[CD_COM_MAX];	/* its Acknowledge's first result byte is the drive status */
	int nparam[CD_COM_MAX];	/* how many parameter bytes it takes */
} Com = {
	{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0,
	 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static volatile u_char *CD_REG0 = (u_char *)CDROM_REG0_ADDR;
static volatile u_char *CD_REG1 = (u_char *)CDROM_REG1_ADDR;
static volatile u_char *CD_REG2 = (u_char *)CDROM_REG2_ADDR;
static volatile u_char *CD_REG3 = (u_char *)CDROM_REG3_ADDR;
static volatile u_long *COM_DELAY = (u_long *)COM_DELAY_ADDR;
static volatile u_short *SPU_REG = (u_short *)SPU_REG_BASE;

/* Last interrupt seen for: [0] the command, [1] data (DataReady, DataEnd), [2] DataEnd alone */
static volatile u_char Intr[3] = {0};

static u_char Result[3][CD_RESULT_SIZE];

static struct {
	int time;	/* VSync count it expires at */
	int cnt;	/* polls so far */
	char *func;	/* waiting routine name */
} Alarm;

static inline void callback(void);

static inline void set_alarm(char *func)
{
	Alarm.time = VSync(-1) + CD_ALARM_VSYNC;
	Alarm.cnt = 0;
	Alarm.func = func;
}

static inline int get_alarm(void)
{
	if (Alarm.time < VSync(-1) || Alarm.cnt++ > CD_ALARM_LIMIT) {
		puts("CD timeout: ");
		printf("%s:(%s) Sync=%s, Ready=%s\n", Alarm.func, CD_comstr[CD_com],
		       CD_intstr[Intr[0]], CD_intstr[Intr[1]]);
		CD_flush();
		return -1;
	}
	return 0;
}

static inline void cpy(u_char *dst, u_char *src, int n)
{
	if (dst)
		while (n--)
			*dst++ = *src++;
}

static int getintr(void)
{
	volatile u_char intr;
	volatile u_char result[CD_RESULT_SIZE];
	int i, nres;
	int err = 0;

	*CD_REG0 = CD_BANK_INTR;
	if ((intr = *CD_REG3 & CD_INT_MASK) == 0)
		return 0;
	while (intr != (*CD_REG3 & CD_INT_MASK))
		intr = *CD_REG3 & CD_INT_MASK;

	for (nres = 0; nres < CD_RESULT_SIZE; nres++) {
		if (!(*CD_REG0 & CD_STAT_RSLRRDY))
			break;
		result[nres] = *CD_REG1;
	}
	for (i = nres; i < CD_RESULT_SIZE; i++)
		result[i] = 0;

	*CD_REG0 = CD_BANK_INTR;
	*CD_REG3 = CD_INT_MASK;
	*CD_REG2 = CD_INT_MASK;

	if (intr != CdlAcknowledge || Com.stat[CD_com]) {
		if (!(CD_status & CdlStatShellOpen) && (result[0] & CdlStatShellOpen))
			CD_nopen++;
		CD_status = result[0];
		CD_status1 = result[1];
		err = CD_status & CD_STAT_ERROR_BITS;
	}
	if (intr == CdlDiskError) {
		if (CD_debug > 0)
			printf("DiskError: ");
		if (CD_debug > 0)
			printf("com=%s,code=(%02x:%02x)\n", CD_comstr[CD_com], CD_status, CD_status1);
	}

	switch (intr) {
	case CdlAcknowledge:
		if (err) {
			Intr[0] = CdlDiskError;
			cpy(Result[0], result, CD_RESULT_SIZE);
			return INTR_FLAG_SYNC;
		}
		if (Com.sync[CD_com]) {
			Intr[0] = CdlAcknowledge;
			cpy(Result[0], result, CD_RESULT_SIZE);
			return INTR_FLAG_ACK;
		}
		Intr[0] = CdlComplete;
		cpy(Result[0], result, CD_RESULT_SIZE);
		return INTR_FLAG_SYNC;

	case CdlComplete:
		Intr[0] = err ? CdlDiskError : CdlComplete;
		cpy(Result[0], result, CD_RESULT_SIZE);
		return INTR_FLAG_SYNC;

	case CdlDataReady:
		if (err && nres == 1)
			err = 0;
		Intr[1] = err ? CdlDiskError : CdlDataReady;
		cpy(Result[1], result, CD_RESULT_SIZE);
		*CD_REG0 = CD_BANK_COMMAND;
		*CD_REG3 = 0;
		return INTR_FLAG_READY;

	case CdlDataEnd:
		Intr[1] = Intr[2] = CdlDataEnd;
		cpy(Result[2], result, CD_RESULT_SIZE);
		cpy(Result[1], result, CD_RESULT_SIZE);
		return INTR_FLAG_READY;

	case CdlDiskError:
		Intr[0] = Intr[1] = CdlDiskError;
		cpy(Result[0], result, CD_RESULT_SIZE);
		cpy(Result[1], result, CD_RESULT_SIZE);
		return INTR_FLAG_SYNC | INTR_FLAG_READY;
	}
	puts("CDROM: unknown intr");
	printf("(%d)\n", intr);
	return 0;
}

static inline void callback(void)
{
	int bank;
	int cb_flags;

	bank = *CD_REG0 & CD_STAT_INDEX_MASK;
	while ((cb_flags = getintr()) != 0) {
		if (cb_flags & INTR_FLAG_READY) {
			if (CD_cbready)
				(*CD_cbready)(Intr[1], Result[1]);
		}
		if (cb_flags & INTR_FLAG_SYNC) {
			if (CD_cbsync)
				(*CD_cbsync)(Intr[0], Result[0]);
		}
	}
	*CD_REG0 = bank;
}

int CD_sync(int mode, u_char *result)
{
	int intr;

	set_alarm("CD_sync");
	for (;;) {
		if (get_alarm())
			return -1;
		if (CheckCallback())
			callback();
		if ((intr = Intr[0]) == CdlComplete || intr == CdlDiskError) {
			Intr[0] = CdlComplete;
			cpy(result, Result[0], CD_RESULT_SIZE);
			return intr;
		}
		if (mode)
			return CdlNoIntr;
	}
}

int CD_ready(int mode, u_char *result)
{
	int intr;

	set_alarm("CD_ready");
	for (;;) {
		if (get_alarm())
			return -1;
		if (CheckCallback())
			callback();
		if ((intr = Intr[2]) != CdlNoIntr) {
			Intr[2] = CdlNoIntr;
			cpy(result, Result[2], CD_RESULT_SIZE);
			return intr;
		}
		if ((intr = Intr[1]) != CdlNoIntr) {
			Intr[1] = CdlNoIntr;
			cpy(result, Result[1], CD_RESULT_SIZE);
			return intr;
		}
		if (mode)
			return CdlNoIntr;
	}
}

int CD_cw(u_char com, u_char *param, u_char *result, int async)
{
	int i;

	if (CD_debug > 1)
		printf("%s...\n", CD_comstr[com]);
	if (Com.nparam[com] && param == 0) {
		if (CD_debug > 0)
			printf("%s: no param\n", CD_comstr[com]);
		return -2;
	}
	CD_sync(0, 0);
	if (com == CdlSetloc)
		for (i = 0; i < 4; i++)
			((u_char *)&CD_pos)[i] = param[i];
	if (com == CdlSetmode)
		CD_mode = param[0];
	Intr[0] = CdlNoIntr;
	if (Com.ready[com])
		Intr[1] = CdlNoIntr;
	*CD_REG0 = CD_BANK_COMMAND;
	for (i = 0; i < Com.nparam[com]; i++)
		*CD_REG2 = param[i];
	CD_com = com;
	*CD_REG1 = com;
	if (async)
		return 0;

	set_alarm("CD_cw");
	while (Intr[0] == CdlNoIntr) {
		if (get_alarm())
			return -1;
		if (CheckCallback())
			callback();
	}
	cpy(result, Result[0], CD_RESULT_SIZE);
	return Intr[0] == CdlDiskError ? -1 : 0;
}

int CD_vol(CdlATV *vol)
{
	*CD_REG0 = CD_BANK_ATV_LEFT;
	*CD_REG2 = vol->val0;
	*CD_REG3 = vol->val1;
	*CD_REG0 = CD_BANK_ATV_RIGHT;
	*CD_REG1 = vol->val2;
	*CD_REG2 = vol->val3;
	*CD_REG3 = CD_ADVC_APPLY;
	return 0;
}

void CD_flush(void)
{
	*CD_REG0 = CD_BANK_INTR;
	while (*CD_REG3 & CD_INT_MASK) {
		*CD_REG0 = CD_BANK_INTR;
		*CD_REG3 = CD_INT_MASK;
		*CD_REG2 = CD_INT_MASK;
	}
	Intr[1] = Intr[2] = CdlNoIntr;
	Intr[0] = CdlComplete;
	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = 0;
	*COM_DELAY = COM_DELAY_DEFAULT;
}

int CD_initvol(void)
{
	CdlATV vol;

	if (SPU_REG[SPU_REG_CUR_MVOL_L] == 0 && SPU_REG[SPU_REG_CUR_MVOL_R] == 0) {
		SPU_REG[SPU_REG_MVOL_L] = SPU_VOL_MAX;
		SPU_REG[SPU_REG_MVOL_R] = SPU_VOL_MAX;
	}
	SPU_REG[SPU_REG_CDVOL_L] = SPU_VOL_MAX;
	SPU_REG[SPU_REG_CDVOL_R] = SPU_VOL_MAX;
	SPU_REG[SPU_REG_SPUCNT] = SPUCNT_CD_INIT;

	vol.val0 = vol.val2 = CD_ATV_UNITY;
	vol.val1 = vol.val3 = 0;
	*CD_REG0 = CD_BANK_ATV_LEFT;
	*CD_REG2 = vol.val0;
	*CD_REG3 = vol.val1;
	*CD_REG0 = CD_BANK_ATV_RIGHT;
	*CD_REG1 = vol.val2;
	*CD_REG2 = vol.val3;
	*CD_REG3 = CD_ADVC_APPLY;
	return 0;
}

void CD_initintr(void)
{
	CD_cbsync = CD_cbready = 0;
	CD_status = CD_status1 = 0;
	ResetCallback();
	InterruptCallback(PSX_IRQ_CDROM, callback);
}

/* CD_init prints this record's address */
static struct {
	volatile u_char *intr;
	u_char *result;
	u_char *com;
	int *status;
	CdlLOC *pos;
	char *id;
} Addr = {
	Intr, Result[0], &CD_com, &CD_status, &CD_pos,
	"$Id: bios.c,v 1.86 1997/03/28 07:42:42 makoto Exp yos $",
};

int CD_init(void)
{
	puts("CD_init:");
	printf("addr=%08x\n", &Addr);
	CD_com = 0;
	CD_mode = 0;
	CD_cbsync = CD_cbready = 0;
	CD_status = CD_status1 = 0;
	ResetCallback();
	InterruptCallback(PSX_IRQ_CDROM, callback);

	*CD_REG0 = CD_BANK_INTR;
	while (*CD_REG3 & CD_INT_MASK) {
		*CD_REG0 = CD_BANK_INTR;
		*CD_REG3 = CD_INT_MASK;
		*CD_REG2 = CD_INT_MASK;
	}
	Intr[1] = Intr[2] = CdlNoIntr;
	Intr[0] = CdlComplete;
	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = 0;
	*COM_DELAY = COM_DELAY_DEFAULT;

	CD_cw(CdlNop, 0, 0, 0);
	if (CD_status & CdlStatShellOpen)
		CD_cw(CdlNop, 0, 0, 0);
	if (CD_cw(CdlReset, 0, 0, 0))
		return -1;
	if (CD_cw(CdlDemute, 0, 0, 0))
		return -1;
	if (CD_sync(0, 0) != CdlComplete)
		return -1;
	return 0;
}

static volatile u_long *CDROM_DELAY = (u_long *)CDROM_DELAY_ADDR;
static volatile u_long *DMA_DPCR = (u_long *)DMA_DPCR_ADDR;
static volatile u_long *DMA3_MADR = (u_long *)DMA3_MADR_ADDR;
static volatile u_long *DMA3_BCR = (u_long *)DMA3_BCR_ADDR;
static volatile u_long *DMA3_CHCR = (u_long *)DMA3_CHCR_ADDR;

int CD_datasync(int mode)
{
	set_alarm("CD_datasync");
	for (;;) {
		if (get_alarm())
			return -1;
		if (*DMA3_CHCR & DMA_CHCR_BUSY) {
			if (mode)
				return 1;
		} else
			return 0;
	}
}

int CD_getsector(void *madr, int size)
{
	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = CD_REQ_BFRD;
	*CDROM_DELAY = CDROM_DELAY_DMA;
	*COM_DELAY = COM_DELAY_DMA;
	*DMA_DPCR |= DMA_DPCR_CDROM_EN;
	*DMA3_MADR = (u_long)madr;
	*DMA3_BCR = size | DMA_BCR_SINGLE_BLOCK;
	while (!(*CD_REG0 & CD_STAT_DRQSTS))
		;
	*DMA3_CHCR = DMA_CHCR_CDROM_READ;
	while (*DMA3_CHCR & DMA_CHCR_BUSY)
		;
	*COM_DELAY = COM_DELAY_DEFAULT;
	return 0;
}

int CD_getsector2(void *madr, int size)
{
	volatile u_long dummy;

	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = CD_REQ_BFRD;
	*CDROM_DELAY = CDROM_DELAY_CHOP;
	*COM_DELAY = COM_DELAY_DEFAULT;
	*DMA_DPCR |= DMA_DPCR_CDROM_EN;
	*DMA3_MADR = (u_long)madr;
	*DMA3_BCR = size | DMA_BCR_SINGLE_BLOCK;
	while (!(*CD_REG0 & CD_STAT_DRQSTS))
		;
	*DMA3_CHCR = DMA_CHCR_CDROM_CHOP;
	dummy = *DMA3_CHCR;
	return 0;
}

void CD_set_test_parmnum(int n)
{
	Com.nparam[CD_CMD_TEST] = n;
}
