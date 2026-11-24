#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

void _spu_setInTransfer(int flag)
{
    if (flag == 1) {
        _spu_inTransfer = 0;
    } else {
        _spu_inTransfer = 1;
    }
}

int _spu_getInTransfer(void)
{
    return _spu_inTransfer != 1;
}
