/*
 * Memory card initialization, start, and stop routines.
 */
#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include "card_int.h"
#include "../api/api_int.h"

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static unsigned char card_id[8] = {'P', 's', 0x03, 0x25, 0x9b, 0x52, 0x43, 0x00};

void InitCARD(long mode)
{
    ChangeClearPAD(0);
    EnterCriticalSection();
    if (!ReadInitPadFlag()) {
        mode = 0;
    }
    InitCARD2(mode);
    _copy_memcard_patch();
    _patch_card();
    _patch_card2();
    ExitCriticalSection();
}

/* returns no value */
long StartCARD(void)
{
    EnterCriticalSection();
    StartCARD2();
    ChangeClearPAD(0);
    ExitCriticalSection();
}

/* returns no value */
long StopCARD(void)
{
    StopCARD2();
    _ExitCard();
}
