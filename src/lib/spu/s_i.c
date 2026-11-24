#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

void SpuInit(void)
{
    _SpuInit(SPU_INIT_COLD);
}
