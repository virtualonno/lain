#ifndef CD_H
#define CD_H

#include <sys/types.h>

/* One part of a disc file: its first sector, counted from the file's first, and its size in bytes. */
typedef struct {
    int sector;
    int size;
} FilePart;

int InitCdRequests(void);
void HandleCdReady(int status, u_char *result);
int PollCdRequest(int handle);

int RequestFilePartRead(int file, int part, FilePart *parts, void *destAddress);

#endif
