/* The game's heap: one area split into blocks, each behind a HeapBlock header, allocated first fit
   and merged with free neighbors when freed. */
#include <sys/types.h>
#include <stdio.h>
#include "heap.h"
#include "host_log.h"

/* The header before each block. Blocks are linked in address order; size counts the bytes after the header. */
typedef struct HeapBlock {
    unsigned int tag;
    unsigned int size;
    struct HeapBlock *next;
    struct HeapBlock *previous;
} HeapBlock;

/* A block's tag: "Free" or "Use!" in ASCII. */
enum {
    HEAP_FREE_TAG = 0x46726565,
    HEAP_USED_TAG = 0x55736521
};

void *gHeapBase = 0;

int gHeapSize = -1;

int gHeapPeakUsed = 0;

int gHeapUsed = 0;

HeapBlock *gHeapSearchStart = 0;

void *AllocHeapBlock(int size)
{
    HeapBlock *block;
    HeapBlock *remainder;
    unsigned int tag;

    if (gHeapBase == 0) {
        printf("allocate fault");
        return 0;
    }
    block = gHeapSearchStart;
    if (size != 0) {
        size += 3U;
        size &= ~3U;
        if (block != 0) {
            do {
                tag = block->tag;
                if (tag == HEAP_USED_TAG) {
                    block = block->next;
                } else if (tag == HEAP_FREE_TAG) {
                    if (block->size >= (unsigned int)size) {
                        if (block->size - size < 32) {
                            size = block->size;
                        } else {
                            remainder = (HeapBlock *)((char *)block + (size + sizeof(*block)));
                            remainder->tag = tag;
                            remainder->size = block->size - size - sizeof(*block);
                            remainder->previous = block;
                            remainder->next = block->next;
                            if (remainder->next != 0) {
                                remainder->next->previous = remainder;
                            }
                            block->next = remainder;
                        }
                        block->tag = HEAP_USED_TAG;
                        block->size = size;
                        gHeapUsed += sizeof(*block) + size;
                        if (gHeapPeakUsed < gHeapUsed) {
                            gHeapPeakUsed = gHeapUsed;
                        }
                        if (block == gHeapSearchStart) {
                            gHeapSearchStart = block->next;
                        }
                        return block + 1;
                    }
                    block = block->next;
                } else {
                    printf("malloc::magic field corrupt\n");
                    return 0;
                }
            } while (block != 0);
        }
    }
    return 0;
}

int FreeHeapBlock(void *address)
{
    HeapBlock *block;
    HeapBlock *neighbor;
    unsigned int tag;

    block = (HeapBlock *)((unsigned long)address - sizeof(*block));
    if (address != 0) {
        tag = block->tag;
        if (tag == HEAP_USED_TAG) {
            gHeapUsed -= block->size;
            neighbor = block->next;
            if (neighbor != 0) {
                if (neighbor->tag == HEAP_FREE_TAG) {
                    block->size += sizeof(*block) + neighbor->size;
                    block->next = neighbor->next;
                    if (neighbor->next != 0) {
                        neighbor->next->previous = block;
                    }
                    gHeapUsed -= sizeof(*block);
                } else if (neighbor->tag != tag) {
                    HostLogPrintf("free::magic field corrupt(next)\n");
                }
            }
            neighbor = block->previous;
            if (neighbor != 0 && neighbor->tag == HEAP_FREE_TAG) {
                neighbor->size += sizeof(*block) + block->size;
                neighbor->next = block->next;
                if (block->next != 0) {
                    block->next->previous = neighbor;
                }
                gHeapUsed -= sizeof(*block);
                if (neighbor < gHeapSearchStart) {
                    gHeapSearchStart = neighbor;
                }
            } else {
                block->tag = HEAP_FREE_TAG;
                if (block < gHeapSearchStart) {
                    gHeapSearchStart = block;
                }
            }
        } else {
            HostLogPrintf("free::magic field corrupt %08x\n", address);
        }
    }
}

int InitHeapArea(void *base, int size)
{
    HeapBlock *block = base;

    gHeapSize = size;
    block->tag = HEAP_FREE_TAG;
    gHeapBase = base;
    block->size = size - sizeof(*block);
    block->next = 0;
    block->previous = 0;
    gHeapSearchStart = block;
    return 1;
}

int GetHeapPeakUsed(void)
{
    return gHeapPeakUsed;
}

int CheckHeapBlock(void *address)
{
    int result = 0;
    /* the header is taken 8 bytes before address; AllocHeapBlock returns addresses 16 bytes past theirs */
    HeapBlock *block = (HeapBlock *)((char *)address - 8);
    unsigned int tag = block->tag;
    HeapBlock *neighbor;

    if (tag != HEAP_FREE_TAG && tag != HEAP_USED_TAG) {
        printf("WARNING:This block may be damaged!\n");
        result = -1;
    }
    neighbor = block->previous;
    if (neighbor != 0) {
        if (neighbor->tag != HEAP_FREE_TAG && neighbor->tag != HEAP_USED_TAG) {
            printf("WARNING:Before block may be damaged!\n");
            result = -1;
        }
    }
    neighbor = block->next;
    if (neighbor != 0) {
        if (neighbor->tag != HEAP_FREE_TAG && neighbor->tag != HEAP_USED_TAG) {
            printf("WARNING:Next block may be damaged!\n");
            result = -1;
        }
    }
    return result;
}

int GetLeftMemory(void)
{
    return gHeapSize - gHeapUsed;
}
