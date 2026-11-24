/* SPU sound RAM memory allocator state. */

#include <sys/types.h>
#include <stddef.h>
#include <libspu.h>

#include "spu_int.h"

long _spu_AllocBlockNum = 0;
long _spu_AllocLastNum = 0;
SpuMemBlock *_spu_memList = NULL;
