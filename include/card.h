#ifndef CARD_H
#define CARD_H

typedef struct {
    char text[31];
} CardTitlePrefix;

typedef union {
    char bytes[512];
    struct {
        char flags[4];
        CardTitlePrefix prefix;
    } title;
} CardHeader;

/* CardSaveData.signature: the bytes "NIAL" in memory, "LAIN" read as a word */
#define CARD_SAVE_SIGNATURE 0x4C41494E

typedef struct {
    u_char state[4];
} CardNodeState;

typedef struct {
    u_long signature;
    u_char name[20];
    int loadCount;
    short currentDisc;
    short gatesOpened;
    short p2NodesOpened;
    short upgradesApplied;
    short endRollCount;
    short cursorColumn;
    short cursorRow;
    short otherDiscCursorColumn;
    short otherDiscCursorRow;
    CardNodeState nodes[716];
} CardSaveData;

typedef struct {
    CardHeader header;
    CardSaveData save;
} CardSave;

typedef union {
    u_char bytes[0x2000];
    CardSave contents;
} CardSaveBlock;

#endif
