/*
 * LIBDS's callback setters: the sync, ready and start callbacks it keeps in GlobalCallback, and the
 * CD-ROM DMA completion callback, which LIBETC holds.
 */
#include <sys/types.h>
#include <libds.h>
#include "etc_int.h"

DslCB GlobalCallback[3];

DslCB DsSyncCallback(DslCB func)
{
    DslCB previous = GlobalCallback[0];
    GlobalCallback[0] = func;
    return previous;
}

DslCB DsReadyCallback(DslCB func)
{
    DslCB previous = GlobalCallback[1];
    GlobalCallback[1] = func;
    return previous;
}

DslCB DsStartCallback(DslCB func)
{
    DslCB previous = GlobalCallback[2];
    GlobalCallback[2] = func;
    return previous;
}

void *DsDataCallback(void (*func)())
{
    return DMACallback(DMA_CDROM, func);
}
