#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <libspu.h>

#include "spu_int.h"

void SpuQuit(void)
{
    if (_spu_isCalled == 1) {
        _spu_isCalled = 0;
        EnterCriticalSection();
        _spu_transferCallback = NULL;
        _spu_IRQCallback = NULL;
        _SpuDataCallback(NULL);
        CloseEvent(_spu_EVdma);
        DisableEvent(_spu_EVdma);
        ExitCriticalSection();
    }
}
