#include <sys/types.h>
#include <libspu.h>
#include "etc_int.h"
#include "psx_hw.h"

void _SpuCallback(SpuIRQCallbackProc callback)
{
    InterruptCallback(PSX_IRQ_SPU, callback);
}
