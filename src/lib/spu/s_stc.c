#include "spu_int.h"

SpuTransferCallbackProc SpuSetTransferCallback(SpuTransferCallbackProc callback)
{
    SpuTransferCallbackProc previous = _spu_transferCallback;

    if (callback != previous) {
        _spu_transferCallback = callback;
    }
    return previous;
}
