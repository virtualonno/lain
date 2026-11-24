#include <sys/types.h>
#include <libapi.h>
#include <libspu.h>

#include "spu_int.h"

long SpuIsTransferCompleted(long flag)
{
    long event;

    if (_spu_trans_mode == SPU_TRANSFER_BY_IO || _spu_inTransfer == 1) {
        return 1;
    }
    event = TestEvent(_spu_EVdma);
    if (flag == SPU_TRANSFER_WAIT) {
        while (event == 0) {
            event = TestEvent(_spu_EVdma);
        }
        _spu_inTransfer = 1;
        return 1;
    }
    if (event == 1) {
        _spu_inTransfer = 1;
    }
    return event;
}
