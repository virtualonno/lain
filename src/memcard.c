/* PlayStation memory-card I/O (libmcrd wrappers and Shift-JIS save-block pack/unpack) together with
 * the kind-8 gate screen (RunGateScreen), joined in one unit by .rdata string order. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libmcrd.h>
#include <rand.h>
#include <memory.h>
#include <strings.h>
#include "special_scenes.h"
#include "about.h"
#include "nodes.h"
#include "graphics.h"
#include "grid_menu.h"
#include "kana_entry.h"
#include "memcard.h"
#include "system_menu.h"
#include "sprite_quad.h"
#include "pad.h"
#include "sound.h"
#include "text.h"

DVECTOR gGateFlightStart;
short gGateFlightActive;

GsSPRITE gGateSprites[6] = {
    { 0x00, 100, 24, 0x78, 0x10, 0x17, 0x00, 0x00, 320, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 88, 48, 0x90, 0x10, 0x17, 0x00, 0x10, 320, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 104, 176, 0x70, 0x20, 0x17, 0x90, 0x00, 320, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 71, 175, 0x38, 0x20, 0x17, 0x00, 0x28, 336, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 193, 175, 0x38, 0x20, 0x17, 0x38, 0x28, 336, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
    { 0x00, 193, 207, 0x38, 0x10, 0x18, 0x00, 0x00, 320, 329, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0 },
};
u_long gGateClut[8] = {
    0x18630000, 0x00003ce6, 0x00000000, 0x00007ded, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

enum GatePhase {
    GATE_PHASE_ASSEMBLE,
    GATE_PHASE_UPLOAD_SLAB,
    GATE_PHASE_FLIGHT,
    GATE_PHASE_WAIT_INPUT,
    GATE_PHASE_DONE
};

void RunGateScreen(void)
{
    short age[44];
    DVECTOR start[44];
    DVECTOR end[44];
    GsSPRITE tiles[44];
    RECT rect;
    SpriteQuad strips[4];
    RECT move;
    int spawned;
    int delay;
    int phase;
    int blink;
    u_short width;
    int i;
    int j;
    int x;
    RECT *source;
    int selected;
    int finished;
    GsSPRITE *tile;

    InitGateScene();
    LoadTimPartAt(25, 448, 256, 320, 329);
    width = UploadTextToVram((u_char *)"press ANY button", 512, 256, TEXT_UPLOAD_SMALL, 1);
    phase = GATE_PHASE_ASSEMBLE;
    blink = phase;
    spawned = 0;
    delay = 0;
    gGateSprites[5].w = width;
    gGateSprites[5].x = 160 - ((unsigned int)gGateSprites[5].w / 2);
    memset(age, 0, sizeof(age));
    for (i = 0; i < 44; i++) {
        tiles[i].attribute = 0;
        tiles[i].tpage = 23;
        tiles[i].cx = 336;
        tiles[i].cy = 329;
        tiles[i].w = 8;
        tiles[i].h = 8;
        tiles[i].u = 160;
        tiles[i].v = 32;
        tiles[i].r = 128;
        tiles[i].g = 128;
        tiles[i].b = 128;
    }
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 1: case 3:
            strips[i].attribute = 0x800000;
            strips[i].x = 160;
            break;
        case 0: case 2:
            strips[i].attribute = 0;
            strips[i].x = 0;
            break;
        }
        strips[i].y = i < 2 ? 40 : 58;
        strips[i].tpage = 23;
        strips[i].clut = 0x5254;
        strips[i].u = 0;
        strips[i].v = 32;
        strips[i].w = 160;
        strips[i].h = 8;
        strips[i].r = 128;
        strips[i].g = 128;
        strips[i].b = 128;
    }
    rect.x = 912;
    rect.y = 89;
    rect.w = 16;
    rect.h = 1;
    LoadImage(&rect, gGateClut);
    rect.x = 896;
    rect.y = 88;
    rect.w = 8;
    rect.h = 88;
    ClearImage(&rect, 0, 0, 0);
    gGateFlightActive = 0;
    while (phase < GATE_PHASE_DONE) {
        source = &move;
        ReadPadEvent();
        BeginFrame();
        source->w = 8;
        source->h = 32;
        for (j = 0; j < 8; j++) {
            for (i = 0, x = 384; i < 7; i++, x += 8) {
                DrawSync(0);
                move.x = (rand() % 6) * 8 + 448;
                source->y = 328;
                MoveImage(&move, x, j * 32 + 256);
            }
        }
        GsSortFastSprite(gGateSprites, &gSubOt[gActiveBuffer], 0);
        SortSpriteQuad(&strips[0], &gSubOt[gActiveBuffer], 0);
        SortSpriteQuad(&strips[1], &gSubOt[gActiveBuffer], 0);
        switch (phase) {
        case GATE_PHASE_ASSEMBLE:
            if (delay == 0 && spawned < 44) {
                do {
                    selected = rand() % 44;
                } while (age[selected] != 0);
                age[selected] = 1;
                delay = 1;
                spawned++;
                rect.x = 488;
                rect.y = 288;
                rect.w = 2;
                rect.h = 8;
                if (rand() & 1) {
                    tiles[selected].u += 8;
                    rect.x += 2;
                }
                MoveImage(&rect, (selected % 4) * 2 + 320, (selected / 4) * 8);
                start[selected].vx = rand() % 312;
                start[selected].vy = rand() % 232;
                end[selected].vx = gGateFlightStart.vx + ((selected % 4) - 2) * 8;
                end[selected].vy = ((selected / 4) - 5) * 8 + (short)(gGateFlightStart.vy - 4);
            } else {
                delay--;
            }
            finished = 0;
            for (i = 0; i < 44; i++) {
                tile = &tiles[i];
                if (age[i] != 0) {
                    if (age[i] < 5) {
                        tile->x = start[i].vx + (end[i].vx - start[i].vx) * age[i] / 4;
                        tile->y = start[i].vy + (end[i].vy - start[i].vy) * age[i] / 4;
                        age[i]++;
                        GsSortFastSprite(tile, &gSubOt[gActiveBuffer], 1);
                    } else if (age[i] != 0) {
                        GsSortFastSprite(tile, &gSubOt[gActiveBuffer], 1);
                        finished++;
                    }
                }
            }
            if (finished == 44) {
                phase++;
            }
            break;
        case GATE_PHASE_UPLOAD_SLAB:
            rect.x = 320;
            rect.w = 8;
            rect.y = 0;
            rect.h = 88;
            MoveImage(&rect, 896, 88);
            phase = GATE_PHASE_FLIGHT;
            gGateFlightActive = 1;
            break;
        case GATE_PHASE_FLIGHT:
            if (gGateFlightActive == 0) {
                gGatesOpened++;
                phase = GATE_PHASE_WAIT_INPUT;
                if (gGatesOpened >= 4) {
                    PlaySound(30);
                }
            }
            break;
        case GATE_PHASE_WAIT_INPUT:
            if (gPadEvent != PAD_EVENT_NONE) {
                phase = GATE_PHASE_DONE;
            }
            if (gGatesOpened >= 4) {
                for (i = 1; i < 5; i++) {
                    GsSortFastSprite(&gGateSprites[i], &gSubOt[gActiveBuffer], 0);
                }
                SortSpriteQuad(&strips[2], &gSubOt[gActiveBuffer], 0);
                SortSpriteQuad(&strips[3], &gSubOt[gActiveBuffer], 0);
            }
            if (blink < 20) {
                GsSortFastSprite(&gGateSprites[5], &gSubOt[gActiveBuffer], 0);
            }
            if (++blink == 40) {
                blink = 0;
            }
            break;
        }
        UpdateGateScene();
        EndSceneFrame();
    }
    DisableFog();
}

unsigned char gWideDigit9[4] = { 130, 88, 0, 0 };
unsigned char gWideDigit8[4] = { 130, 87, 0, 0 };
unsigned char gWideDigit7[4] = { 130, 86, 0, 0 };
unsigned char gWideDigit6[4] = { 130, 85, 0, 0 };
unsigned char gWideDigit5[4] = { 130, 84, 0, 0 };
unsigned char gWideDigit4[4] = { 130, 83, 0, 0 };
unsigned char gWideDigit3[4] = { 130, 82, 0, 0 };
unsigned char gWideDigit2[4] = { 130, 81, 0, 0 };
unsigned char gWideDigit1[4] = { 130, 80, 0, 0 };
unsigned char gWideDigit0[4] = { 130, 79, 0, 0 };
char gWideSpace[4] = { 0x81, 0x40, 0x00, 0x00 };

int InitMemCard(void)
{
    MemCardInit(0);
    MemCardStart();
    return 0;
}

int CreateMemCardFile(void)
{
    return 0 - MemCardCreateFile(0, "BISLPS-01603LAINdata", 1);
}

int FormatMemCard(void)
{
    return 0 - MemCardFormat(0);
}

int UnformatMemCard(void)
{
    return 0 - MemCardUnformat(0);
}

void CloseMemCardFile(void)
{
    MemCardClose();
}

int OpenMemCardFile(int forWrite)
{
    int flag;

    if (forWrite == 0) {
        flag = 1;
    } else {
        flag = 2;
    }
    return 0 - MemCardOpen(0, "BISLPS-01603LAINdata", flag);
}

int AcceptMemCardAndWait(void)
{
    long command;
    long result;
    int status;

    if (MemCardAccept(0) != 0) {
        while (MemCardSync(1, &command, &result) != 1) {
        }
        status = -result;
    } else {
        status = 0;
    }
    return status;
}

void AcceptMemCard(void)
{
    MemCardAccept(0);
}

void ReadMemCardFile(void *buffer, long bytes)
{
    MemCardReadData(buffer, 0, bytes);
}

void WriteMemCardFile(void *buffer, long bytes)
{
    MemCardWriteData(buffer, 0, bytes);
}

int PollMemCardSync(void)
{
    long command;
    long result;

    if (MemCardSync(1, &command, &result) == 1) {
        if (result == 0)
            return 1;
        else
            return -result;
    }
    return 0;
}

void AppendWideNumber(char *buf, int value, int pad)
{
    int digit;

    if (value >= 10000) {
        value = 9999;
    }
    digit = value / 1000;
    if (digit != 0) {
        strcat(buf, gWideDigits[digit]);
    }
    digit = value % 1000 / 100;
    if (digit != 0) {
        strcat(buf, gWideDigits[digit]);
    } else if (pad != 0) {
        strcat(buf, gWideSpace);
    }
    digit = value % 100 / 10;
    if (digit != 0) {
        strcat(buf, gWideDigits[digit]);
    } else if (pad != 0) {
        strcat(buf, gWideSpace);
    }
    digit = value % 10;
    strcat(buf, gWideDigits[digit]);
}

char *gWideDigits[10] = {
    (void *)&gWideDigit0, (void *)&gWideDigit1, (void *)&gWideDigit2, (void *)&gWideDigit3,
    (void *)&gWideDigit4, (void *)&gWideDigit5, (void *)&gWideDigit6, (void *)&gWideDigit7,
    (void *)&gWideDigit8, (void *)&gWideDigit9,
};
CardHeader gCardHeaderTemplate = {
    {
        83, 67, 19, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 113, 29, 120, 66, 28, 91, 8, 0, 13, 0,
        122, 41, 103, 69, 112, 98, 22, 115, 30, 120, 0, 72, 21, 119, 221, 123, 192, 117,
        0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 240, 79, 84, 69, 244, 15, 0,
        0, 79, 84, 85, 85, 69, 244, 0, 240, 84, 85, 85, 85, 85, 69, 15, 240, 101,
        86, 101, 101, 101, 85, 15, 240, 54, 85, 53, 54, 83, 83, 15, 64, 86, 33, 85,
        86, 86, 86, 4, 79, 69, 255, 81, 81, 255, 85, 244, 95, 69, 51, 83, 83, 53,
        84, 245, 95, 21, 251, 49, 179, 31, 81, 245, 95, 21, 255, 61, 243, 223, 65, 254,
        95, 21, 251, 60, 179, 207, 177, 254, 79, 21, 51, 51, 49, 51, 81, 246, 240, 245,
        33, 51, 51, 18, 95, 246, 0, 15, 79, 33, 18, 244, 64, 245, 0, 0, 0, 255,
        255, 0, 240, 5, 0, 0, 0, 255, 255, 0, 0, 0, 0, 240, 79, 84, 69, 244,
        15, 0, 0, 79, 84, 85, 85, 69, 244, 0, 240, 84, 85, 85, 85, 85, 69, 15,
        240, 101, 86, 101, 101, 101, 85, 15, 240, 54, 85, 53, 54, 83, 83, 15, 64, 86,
        33, 85, 86, 86, 86, 4, 79, 69, 50, 81, 81, 85, 85, 244, 95, 69, 244, 81,
        83, 31, 84, 245, 95, 21, 51, 51, 51, 51, 81, 245, 95, 21, 51, 50, 35, 51,
        65, 254, 95, 21, 255, 63, 243, 255, 177, 254, 79, 21, 51, 51, 49, 51, 81, 246,
        240, 245, 33, 51, 51, 18, 95, 246, 0, 15, 79, 33, 18, 244, 64, 245, 0, 0,
        0, 255, 255, 0, 240, 5, 0, 0, 0, 255, 255, 0, 0, 0, 0, 240, 79, 84,
        69, 244, 15, 0, 0, 79, 84, 85, 85, 69, 244, 0, 240, 84, 85, 85, 85, 85,
        69, 15, 240, 101, 86, 101, 101, 101, 85, 15, 240, 54, 85, 53, 54, 83, 83, 15,
        64, 86, 33, 85, 86, 86, 86, 4, 79, 69, 244, 81, 81, 68, 85, 244, 95, 69,
        49, 83, 83, 21, 84, 245, 95, 21, 51, 50, 35, 51, 81, 245, 95, 21, 251, 60,
        179, 207, 65, 254, 95, 21, 255, 60, 243, 207, 177, 254, 79, 21, 51, 51, 49, 51,
        81, 246, 240, 245, 33, 51, 51, 18, 95, 246, 0, 15, 79, 33, 18, 244, 64, 245,
        0, 0, 0, 255, 255, 0, 240, 5,
    },
};

/* Shift-JIS memory-card title prefixes ("連続ゲイム小説［ｌａｉｎ］　第", 1-in-70 "ゲノム" variant)
 * and stat labels ("ときめき度", "はるまげ度", "がっくり度"; "毒電波" is inline in PackCardSaveBlock). */
const CardTitlePrefix gCardTitlePrefix = { "\230\101\221\261\203\121\203\103\203\200\217\254\220\340\201\155\202\214\202\201\202\211\202\216\201\156\201\100\221\346" };
const CardTitlePrefix gCardTitlePrefixRare = { "\230\101\221\261\203\121\203\155\203\200\217\254\220\340\201\155\202\214\202\201\202\211\202\216\201\156\201\100\221\346" };
const char gTokimekiLabel[12] = "\202\306\202\253\202\337\202\253\223\170";
const char gHarumageLabel[12] = "\202\315\202\351\202\334\202\260\223\170";
const char gGakkuriLabel[12] = "\202\252\202\301\202\255\202\350\223\170";

enum CardTitleStat {
    CARD_STAT_TOKIMEKI,
    CARD_STAT_DOKUDENPA,
    CARD_STAT_HARUMAGE,
    CARD_STAT_GAKKURI
};

int PackCardSaveBlock(CardSaveBlock *block)
{
    char *title;
    int i, j;
    int pages;
    u_char *first, *last;
    int offset;
    const u_char *nodes;
    u_char *checksums;
    int *loadCount;
    short *counts;

    memset(block, 0, sizeof(*block));
    block->contents.header = gCardHeaderTemplate;
    if (rand() % 70 != 0) {
        block->contents.header.title.prefix = gCardTitlePrefix;
    } else {
        block->contents.header.title.prefix = gCardTitlePrefixRare;
    }
    title = &block->contents.header.bytes[4];
    AppendWideNumber(title, gLoadCount + 1, 0);
    strcat(title, "\x89\xf1\x81\x46");
    switch (rand() % 4) {
    case CARD_STAT_TOKIMEKI:
        strcat(title, gTokimekiLabel);
        AppendWideNumber(title, gTokimekiPercent, 1);
        strcat(title, "\x81\x93");
        break;
    case CARD_STAT_DOKUDENPA:
        if (gDokudenpaCount != 0) {
            strcat(title, "\x93\xc5\x93\x64\x94\x67");
            AppendWideNumber(title, gDokudenpaCount, 1);
            strcat(title, "\x82\xc2");
            break;
        }
        /* fall through */
    case CARD_STAT_HARUMAGE:
        title = &block->contents.header.bytes[4];
        strcat(title, gHarumageLabel);
        AppendWideNumber(title, gHarumagePercent, 1);
        strcat(title, "\x81\x93");
        break;
    case CARD_STAT_GAKKURI:
        strcat(title, gGakkuriLabel);
        AppendWideNumber(title, gGakkuriPercent, 1);
        strcat(title, "\x81\x93");
        break;
    }
    block->contents.save.signature = CARD_SAVE_SIGNATURE;
    memcpy(block->bytes + 516, gPlayerName, sizeof(block->contents.save.name));
    loadCount = &block->contents.save.loadCount;
    *loadCount = gLoadCount;
    counts = &block->contents.save.currentDisc;
    *counts++ = gCurrentDisc;
    *counts++ = gGatesOpened;
    *counts++ = gP2NodesOpened;
    *counts++ = gUpgradesApplied;
    *counts++ = gEndRollCount;
    *counts++ = gCursorColumn;
    *counts++ = gCursorRow;
    *counts++ = gOtherDiscCursorColumn;
    *counts++ = gOtherDiscCursorRow;
    pages = sizeof(block->contents.header) / 128;
    first = block->bytes + 558;
    i = 0;
    nodes = (const u_char *)gNodes;
    offset = 0;
    last = block->bytes + 561;
    for (; i < 716; i++) {
        const u_char *node = (const u_char *)(offset + (unsigned long)nodes);
        node += 34;
        *first = node[0];
        first += 4;
        (last + i * 4)[-2] = node[1];
        (last + i * 4)[-1] = node[2];
        (last + i * 4)[0] = node[3];
        offset += sizeof(gNodes[0]);
    }
    pages += (sizeof(block->contents.save) + 127) / 128;
    i = 0;
    if (i < pages) {
        checksums = (u_char *)((pages * 128) + (unsigned long)block->bytes);
        do {
            u_char *page = (u_char *)((i * 128) + (unsigned long)block->bytes);
            int checksum = 0;
            int nextChecksum;
            j = 127;
            do {
                nextChecksum = checksum ^ *page++;
                checksum = nextChecksum;
            } while (--j >= 0);
            checksums[i] = nextChecksum;
        } while (++i < pages);
    }
    return pages + 1;
}

typedef struct {
    u_char bytes[20];
} CardSaveName;

int UnpackCardSaveBlock(CardSaveBlock *block)
{
    int pages;
    int i, j;
    u_char *checksums;
    u_char *page;
    u_char *first, *last;
    int offset;
    u_char *nodes;
    short *counts;

    pages = sizeof(block->contents.header) / 128 + (sizeof(block->contents.save) + 127) / 128;
    i = 0;
    checksums = block->bytes + pages * 128;
    page = block->bytes;
    do {
        u_char *cursor = page;
        u_char expected = checksums[i];
        int checksum = 0;
        j = 127;
        do {
            checksum ^= *cursor++;
        } while (--j >= 0);
        if (expected != (checksum & 0xff)) {
            return 0;
        }
        page += 128;
    } while (++i < pages);

    if (block->contents.save.signature != CARD_SAVE_SIGNATURE) {
        return 0;
    }
    *(CardSaveName *)gPlayerName = *(CardSaveName *)block->contents.save.name;
    gLoadCount = block->contents.save.loadCount;
    counts = &block->contents.save.currentDisc;
    gCurrentDisc = *counts++;
    gGatesOpened = *counts++;
    gP2NodesOpened = *counts++;
    gUpgradesApplied = *counts++;
    gEndRollCount = *counts++;
    gCursorColumn = *counts++;
    gCursorRow = *counts++;
    gOtherDiscCursorColumn = *counts++;
    gOtherDiscCursorRow = *counts++;
    first = block->bytes + 558;
    i = 0;
    nodes = (u_char *)gNodes;
    offset = 0;
    last = block->bytes + 561;
    for (; i < 716; i++) {
        u_char *node = (u_char *)(offset + (unsigned long)nodes);
        node += 34;
        node[0] = *first;
        first += 4;
        node[1] = (last + i * 4)[-2];
        offset += sizeof(gNodes[0]);
        node[2] = (last + i * 4)[-1];
        node[3] = (last + i * 4)[0];
    }
    if ((unsigned int)gLoadCount < 9999) {
        gLoadCount++;
    }
    gShowAllTitles = 0;
    return 1;
}
