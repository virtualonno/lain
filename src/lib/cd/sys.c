/*
 * Public CD-ROM control, status, callback, and position-conversion routines.
 */
#include <sys/types.h>
#include <libcd.h>
#include "cd_int.h"
#include "etc_int.h"

/* Commands whose parameter is a CdlLOC, sent via CdlSetloc first */
static int CD_setloc[CD_COM_MAX] = {
	0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
};

static inline int cd_control(u_char com, u_char *param, u_char *result, int async)
{
	CdlCB old = CD_cbsync;
	int retries = 3;

	do {
		CD_cbsync = 0;
		if (com != CdlNop && ((u_char)CD_status & CdlStatShellOpen))
			CD_cw(CdlNop, 0, 0, 0);
		if (param == 0 || CD_setloc[com] == 0 || CD_cw(CdlSetloc, param, result, 0) == 0) {
			CD_cbsync = old;
			if (CD_cw(com, param, result, async) == 0)
				return 0;
		}
	} while (--retries != -1);
	CD_cbsync = old;
	return -1;
}

int CdStatus(void)
{
	return (u_char)CD_status;
}

int CdMode(void)
{
	return CD_mode;
}

int CdLastCom(void)
{
	return CD_com;
}

CdlLOC *CdLastPos(void)
{
	return &CD_pos;
}

int CdReset(int mode)
{
	if (mode == 2) {
		CD_initintr();
		return 1;
	}
	if (CD_init())
		return 0;
	if (mode == 1 && CD_initvol())
		return 0;
	return 1;
}

void CdFlush(void)
{
	CD_flush();
}

int CdSetDebug(int level)
{
	int old = CD_debug;

	CD_debug = level;
	return old;
}

char *CdComstr(u_char com)
{
	if (com > CdlReadS)
		return "none";
	return CD_comstr[com];
}

char *CdIntstr(u_char intr)
{
	if (intr > 6)
		return "none";
	return CD_intstr[intr];
}

int CdSync(int mode, u_char *result)
{
	return CD_sync(mode, result);
}

int CdReady(int mode, u_char *result)
{
	return CD_ready(mode, result);
}

CdlCB CdSyncCallback(CdlCB func)
{
	CdlCB old = CD_cbsync;

	CD_cbsync = func;
	return old;
}

CdlCB CdReadyCallback(CdlCB func)
{
	CdlCB old = CD_cbready;

	CD_cbready = func;
	return old;
}

int CdControl(u_char com, u_char *param, u_char *result)
{
	return cd_control(com, param, result, 0) == 0;
}

int CdControlF(u_char com, u_char *param)
{
	return cd_control(com, param, 0, 1) == 0;
}

int CdControlB(u_char com, u_char *param, u_char *result)
{
	if (cd_control(com, param, result, 0))
		return 0;
	return CD_sync(0, result) == CdlComplete;
}

int CdMix(CdlATV *vol)
{
	CD_vol(vol);
	return 1;
}

int CdGetSector(void *madr, int size)
{
	return CD_getsector(madr, size) == 0;
}

int CdGetSector2(void *madr, int size)
{
	return CD_getsector2(madr, size) == 0;
}

void *CdDataCallback(void (*func)())
{
	return DMACallback(DMA_CDROM, func);
}

int CdDataSync(int mode)
{
	return CD_datasync(mode);
}

CdlLOC *CdIntToPos(int i, CdlLOC *p)
{
	int min;

	i += 150;
	p->sector = itob(i % 75);
	i /= 75;
	min = i / 60;
	p->second = itob(i % 60);
	p->minute = itob(min);
	return p;
}

int CdPosToInt(CdlLOC *p)
{
	return (btoi(p->minute) * 60 + btoi(p->second)) * 75 + btoi(p->sector) - 150;
}
