#ifndef SPU_MALLOC_H
#define SPU_MALLOC_H

/* One record of the SPU RAM allocator's table (SPU_MALLOC_RECSIZ bytes); addr's top bits tag the
 * record, SPU_MALLOC_START_ADDR is where SpuInitMalloc's first block starts. */
#define SPU_MALLOC_FREE         0x80000000
#define SPU_MALLOC_END          0x40000000
#define SPU_MALLOC_EMPTY        0x2fffffff
#define SPU_MALLOC_ADDR         0x0fffffff
#define SPU_MALLOC_START_ADDR   0x1010

typedef struct {
    unsigned long addr;
    unsigned long size;
} SpuMemBlock;

#endif
