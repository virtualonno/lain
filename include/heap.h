#ifndef HEAP_H
#define HEAP_H

void *AllocHeapBlock(int size);
int FreeHeapBlock(void *address);
int InitHeapArea(void *base, int size);
int GetLeftMemory(void);

#endif
