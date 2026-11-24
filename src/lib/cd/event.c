/*
 * CD-ROM subsystem initialization and default kernel event callbacks.
 */
#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <stdio.h>
#include "cd_int.h"

CdlCB CD_cbread = 0;
int CD_read_dma_mode = 0;

static int init_cd_hardware(void);
static void def_cbsync(u_char intr, u_char *result);
static void def_cbready(u_char intr, u_char *result);
static void def_cbread(u_char intr, u_char *result);

int CdInit(void)
{
	int retries = 4;

	while (init_cd_hardware() != 1) {
		if (retries-- == 0) {
			printf("CdInit: Init failed\n");
			return 0;
		}
	}
	CD_cbsync = def_cbsync;
	CD_cbready = def_cbready;
	CD_cbread = def_cbread;
	CD_read_dma_mode = 0;
	return 1;
}

static int init_cd_hardware(void)
{
	if (CD_init())
		return 0;
	return CD_initvol() == 0;
}

static void def_cbsync(u_char intr, u_char *result)
{
	DeliverEvent(HwCdRom, EvSpCOMP);
}

static void def_cbready(u_char intr, u_char *result)
{
	DeliverEvent(HwCdRom, EvSpDR);
}

static void def_cbread(u_char intr, u_char *result)
{
	DeliverEvent(HwCdRom, EvSpDR);
}
