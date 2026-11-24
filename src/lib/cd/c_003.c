/* StUnSetRing: clear the data and ready callbacks (libds's when DS_active is 1, otherwise libcd's) and
 * stop sector data requests. */

#include <sys/types.h>
#include <libapi.h>
#include <libcd.h>
#include <libds.h>

#define CD_STREAM_MODULE
#include "cd_int.h"

void StUnSetRing(void)
{
	EnterCriticalSection();
	if (DS_active == 1) {
		DsDataCallback(0);
		DsReadyCallback(0);
	} else {
		CdDataCallback(0);
		CdReadyCallback(0);
	}
	*CD_REG0 = CD_BANK_COMMAND;
	*CD_REG3 = CD_REQ_CLEAR;	/* bank 0's request register: clearing BFRD stops sector data requests */
	ExitCriticalSection();
}
