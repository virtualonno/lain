/* CD-ROM streaming read setup and ready-interrupt callback adapter. */

#include <sys/types.h>
#include <libcd.h>
#include "cd_int.h"

u_long StMode;

static void st_cbready(u_char intr, u_char *result);

int CdRead2(long mode)
{
	u_char param[4];

	param[0] = mode;
	CdControl(CdlSetmode, param, 0);
	if (mode & CdlModeStream) {
		if (mode & CdlModeSize1)
			StMode = ST_MODE_2340;
		else
			StMode = ST_MODE_2048;
		CdDataCallback(data_ready_callback);
		CdReadyCallback(st_cbready);
	}
	return CdControl(CdlReadS, 0, 0);
}

static void st_cbready(u_char intr, u_char *result)
{
	StCdInterrupt();
}
