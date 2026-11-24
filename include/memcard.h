#ifndef MEMCARD_H
#define MEMCARD_H

#include <sys/types.h>
#include <libgte.h>
#include "card.h"

extern DVECTOR gGateFlightStart;
extern short gGateFlightActive;
extern unsigned char gWideDigit9[4];
extern unsigned char gWideDigit8[4];
extern unsigned char gWideDigit7[4];
extern unsigned char gWideDigit6[4];
extern unsigned char gWideDigit5[4];
extern unsigned char gWideDigit4[4];
extern unsigned char gWideDigit3[4];
extern unsigned char gWideDigit2[4];
extern unsigned char gWideDigit1[4];
extern unsigned char gWideDigit0[4];
extern char *gWideDigits[10];

/* Shared by UpdateSystemMenu (MENU_LOAD, gMemCardStep) and RunStartScreen (sub). */
enum MemCardLoadStep {
    MEMCARD_LOAD_ACCEPT,
    MEMCARD_LOAD_POLL_ACCEPT,
    MEMCARD_LOAD_POLL_READ,
    MEMCARD_LOAD_DONE,
    MEMCARD_LOAD_ERR_CORRUPT,
    MEMCARD_LOAD_ERR_NO_CARD = 10,
    MEMCARD_LOAD_ERR_INVALID = 20,
    MEMCARD_LOAD_ERR_UNFORMATTED = 30,
    MEMCARD_LOAD_ERR_NO_FILE = 40,
    MEMCARD_LOAD_OPEN = 100,
    MEMCARD_LOAD_START_READ
};

void RunGateScreen(void);
int InitMemCard(void);
int CreateMemCardFile(void);
int FormatMemCard(void);
void CloseMemCardFile(void);
int OpenMemCardFile(int forWrite);
void AcceptMemCard(void);
void ReadMemCardFile(void *buffer, long bytes);
void WriteMemCardFile(void *buffer, long bytes);
int PollMemCardSync(void);
int PackCardSaveBlock(CardSaveBlock *block);
int UnpackCardSaveBlock(CardSaveBlock *block);

#endif
