/* The disc read queue: up to 24 requests, read one at a time by LIBCD's sync and ready callbacks. */
#include <sys/types.h>
#include <libcd.h>
#include <libapi.h>

#include "cd.h"
#include "media_player.h"

#define CD_REQUEST_COUNT 24

typedef struct {
    CdlFILE file; /* pos set by RequestNamedFileRead; nothing reads it */
    int done;
    int busy;
    int ahead; /* requests queued before this one */
    int sector;
    int size;
    void *dest;
} CdRequest;

static CdRequest sCdRequests[CD_REQUEST_COUNT];
/* Retries and queued requests enter CD_READ_WAIT_SETMODE after CdlSetloc once drive mode is set. */
enum CdReadState {
    CD_READ_IDLE,
    CD_READ_WAIT_SETLOC,
    CD_READ_WAIT_SETMODE,
    CD_READ_WAIT_READ,
    CD_READ_READING
};
/* Shared with HandleCdSync (CdSyncCallback) and HandleCdReady (CdReadyCallback). */
volatile int gCdReadState = CD_READ_IDLE;
volatile int gCdDiskErrorCount = 0; /* incremented by HandleCdSync */
volatile int gCdUnreadWord = 0; /* 4 bytes; zeroed by HandleCdSync when CdlReadN is acknowledged, nothing reads it */
volatile int gCdReadyCount = 0; /* incremented by HandleCdReady, zeroed by HandleCdSync */
volatile int gCdPendingCount = 0; /* decremented by HandleCdSync */
int gCdSeekSector = 0;
int gCdLastSector = 0;
int gCdCurrentRequest = 0;
void *gCdReadDest;
volatile int gCdReadRemaining; /* decremented by HandleCdReady, read by HandleCdSync */

static __inline__ void IssueCdSetloc(int sector)
{
    CdlLOC loc;

    CdIntToPos(sector, &loc);
    CdControlF(CdlSetloc, (u_char *)&loc);
}

static __inline__ int SearchFileSector(char *name, int *size)
{
    CdlFILE file;

    if (CdSearchFile(&file, name) != 0) {
        *size = file.size;
        return CdPosToInt(&file.pos);
    }
    return -1;
}

/* LIBCD calls both callbacks with the interrupt byte zero-extended in the argument word. */
void HandleCdSync(int status, u_char *result)
{
    u_char modeArg[2];
    int i;
    int nextRequest;

    if (status == CdlDiskError) {
        if (gCdReadState == CD_READ_READING) {
            CdControlF(CdlPause, 0);
            ++gCdDiskErrorCount;
        } else {
            gCdLastSector = 0;
            ++gCdDiskErrorCount;
            gCdSeekSector = sCdRequests[gCdCurrentRequest].sector;
            gCdReadState = CD_READ_WAIT_SETLOC;
            IssueCdSetloc(gCdSeekSector);
            gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
            gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
        }
    } else if (status == CdlComplete) {
        switch (gCdReadState) {
        case CD_READ_WAIT_SETLOC:
            modeArg[0] = CdlModeSpeed | CdlModeSize1;
            CdControlF(CdlSetmode, modeArg);
            gCdReadState++;
            break;
        case CD_READ_WAIT_SETMODE:
            CdReadyCallback((CdlCB)HandleCdReady);
            CdControlF(CdlReadN, 0);
            gCdReadState++;
            break;
        case CD_READ_WAIT_READ:
            ++gCdReadState;
            gCdUnreadWord = 0;
            gCdReadyCount = 0;
            break;
        case CD_READ_READING:
            if (gCdReadRemaining == 0) {
                gCdPendingCount--;
                sCdRequests[gCdCurrentRequest].done = 1;
                if (gCdPendingCount != 0) {
                    for (i = 0; i < CD_REQUEST_COUNT; i++) {
                        if (sCdRequests[i].busy && !sCdRequests[i].done) {
                            if (--sCdRequests[i].ahead == 0) {
                                nextRequest = i;
                            }
                        }
                    }
                    gCdCurrentRequest = nextRequest;
                    gCdLastSector = 0;
                    gCdSeekSector = sCdRequests[gCdCurrentRequest].sector;
                    IssueCdSetloc(gCdSeekSector);
                    gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
                    gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
                    gCdReadState = CD_READ_WAIT_SETMODE;
                } else {
                    gCdReadState = CD_READ_IDLE;
                    CdSyncCallback(0);
                }
            } else {
                gCdReadState = CD_READ_WAIT_SETMODE;
                gCdLastSector = 0;
                gCdSeekSector = sCdRequests[gCdCurrentRequest].sector;
                IssueCdSetloc(gCdSeekSector);
                gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
                gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
            }
            break;
        }
    }
}

int RequestFileRead(int file, void *dest)
{
    int i;
    CdRequest *req;
    CdlLOC loc;

    for (i = 0; i < CD_REQUEST_COUNT; i++) {
        if (sCdRequests[i].busy == 0) {
            break;
        }
    }
    if (i == CD_REQUEST_COUNT) {
        return -1;
    }
    EnterCriticalSection();
    req = &sCdRequests[i];
    req->sector = gDiscFiles[file].sector;
    req->size = gDiscFiles[file].size;
    req->dest = dest;
    req->ahead = gCdPendingCount;
    gCdPendingCount++;
    req->busy = 1;
    if (gCdReadState == CD_READ_IDLE) {
        CdSyncCallback((CdlCB)HandleCdSync);
        gCdReadState = CD_READ_WAIT_SETLOC;
        gCdCurrentRequest = i;
        gCdLastSector = 0;
        gCdSeekSector = req->sector;
        CdIntToPos(gCdSeekSector, &loc);
        CdControlF(CdlSetloc, (u_char *)&loc);
        gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
        gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
    }
    ExitCriticalSection();
    return i;
}

int RequestFilePartRead(int file, int part, FilePart *parts, void *destAddress)
{
    int i;
    CdRequest *req;
    CdlLOC loc;

    for (i = 0; i < CD_REQUEST_COUNT; i++) {
        if (sCdRequests[i].busy == 0) {
            break;
        }
    }
    if (i == CD_REQUEST_COUNT) {
        return -1;
    }
    EnterCriticalSection();
    req = &sCdRequests[i];
    req->sector = gDiscFiles[file].sector + parts[part].sector;
    req->size = parts[part].size;
    req->dest = destAddress;
    req->ahead = gCdPendingCount;
    gCdPendingCount++;
    req->busy = 1;
    if (gCdReadState == CD_READ_IDLE) {
        gCdReadState = CD_READ_WAIT_SETLOC;
        gCdCurrentRequest = i;
        CdSyncCallback((CdlCB)HandleCdSync);
        gCdLastSector = 0;
        gCdSeekSector = sCdRequests[gCdCurrentRequest].sector;
        CdIntToPos(gCdSeekSector, &loc);
        CdControlF(CdlSetloc, (u_char *)&loc);
        gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
        gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
    }
    ExitCriticalSection();
    return i;
}

/* The found position goes to file.pos, not sector: the seek is to the sector the slot last held. */
int RequestNamedFileRead(char *name, void *dest)
{
    int i;
    int pos;
    int bytes;
    CdRequest *req;

    for (i = 0; i < CD_REQUEST_COUNT; i++) {
        if (sCdRequests[i].busy == 0) {
            break;
        }
    }
    if (i == CD_REQUEST_COUNT) {
        return -1;
    }
    pos = SearchFileSector(name, &bytes);
    if (pos < 0) {
        return -1;
    }
    req = &sCdRequests[i];
    CdIntToPos(pos, &req->file.pos);
    req->ahead = gCdPendingCount;
    gCdPendingCount++;
    req->busy = 1;
    req->dest = dest;
    req->size = bytes;
    if (gCdReadState == CD_READ_IDLE) {
        CdSyncCallback((CdlCB)HandleCdSync);
        gCdReadState = CD_READ_WAIT_SETLOC;
        gCdCurrentRequest = i;
        gCdLastSector = 0;
        gCdSeekSector = req->sector;
        IssueCdSetloc(gCdSeekSector);
        gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
        gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
    }
    return i;
}

int InitCdRequests(void)
{
    int i;

    for (i = 0; i < CD_REQUEST_COUNT; i++) {
        sCdRequests[i].busy = 0;
        sCdRequests[i].done = 0;
    }
    return CdInit();
}

void HandleCdReady(int status, u_char *result)
{
    union {
        CdlLOC loc;
        u_long words[3];
    } header;
    int sector;
    int readError = 0;

    gCdReadyCount++;
    if (status == CdlDataReady) {
        CdGetSector(&header, 3);
        sector = CdPosToInt(&header.loc);
        if (gCdLastSector == 0) {
            readError = gCdSeekSector != sector;
        } else if (sector != gCdLastSector + 1) {
            readError = 1;
        }
        gCdLastSector = sector;
        if (gCdReadRemaining >= 2048) {
            CdGetSector(gCdReadDest, 512);
            gCdReadDest = (char *)gCdReadDest + 2048;
            gCdReadRemaining -= 2048;
        } else {
            CdGetSector(gCdReadDest, (gCdReadRemaining + 3) / 4);
            gCdReadRemaining = 0;
        }
        if (gCdReadRemaining == 0) {
            CdReadyCallback(0);
            CdControlF(CdlPause, 0);
        }
    } else {
        readError = 1;
    }
    if (readError) {
        gCdReadRemaining = -1;
        CdReadyCallback(0);
        CdControlF(CdlPause, 0);
    }
}

void SeekCurrentRequest(void)
{
    CdlLOC loc;

    gCdLastSector = 0;
    gCdSeekSector = sCdRequests[gCdCurrentRequest].sector;
    CdIntToPos(gCdSeekSector, &loc);
    CdControlF(CdlSetloc, (u_char *)&loc);
    gCdReadRemaining = sCdRequests[gCdCurrentRequest].size;
    gCdReadDest = sCdRequests[gCdCurrentRequest].dest;
}

int FindFileSector(char *name, int *size)
{
    CdlFILE file;
    int sector;

    if (CdSearchFile(&file, name) == 0) {
        sector = -1;
    } else {
        *size = file.size;
        sector = CdPosToInt(&file.pos);
    }
    return sector;
}

int PollCdRequest(int handle)
{
    if (sCdRequests[handle].busy != 0) {
        if (sCdRequests[handle].done != 0) {
            sCdRequests[handle].busy = 0;
            sCdRequests[handle].done = 0;
            return 1;
        }
        return 0;
    }
    return -1;
}
