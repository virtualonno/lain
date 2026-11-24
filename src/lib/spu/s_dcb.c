/*
 * LIBSPU DMA transfer callback registration.
 */
#include <sys/types.h>
#include <libspu.h>
#include "etc_int.h"
#include "spu_int.h"

void _SpuDataCallback(void (*func)(void))
{
    DMACallback(DMA_SPU, func);
}
