/* LAPKS.BIN animation clip manager: schedules async CD reads into idle and 6-slot sequence pools
 * (ClipSlot.state, enum ClipSlotState) and owns the 848 KiB heap arena sHeapArea. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <rand.h>
#include "loading.h"
#include "cd.h"
#include "heap.h"
#include "file_parts.h"
#include "lapks_player.h"
#include "clip.h"
#include "grid_menu.h"
#include "graphics.h"
#include "grid_nodes.h"
#include "sound.h"

enum ClipSlotState {
    CLIP_SLOT_FAILED = -1,
    CLIP_SLOT_FREE,
    CLIP_SLOT_READING,
    CLIP_SLOT_READY
};

typedef struct {
    void *buffer;
    short request;
    short unused; /* zeroed at init; nothing reads it */
    short state;
    short soundProgram;
    short playCount;
} ClipSlot;

int gPlayingIdleSlot;
int gNextSequenceSlot;
short gLoadedOpenClipSet;

static ClipSlot sDefaultIdleSlot;
static ClipSlot sIdleSlots[1];
static ClipSlot sSequenceSlots[6];
static int sIdleLoadHolds[6]; /* one per sequence slot; all -1, never set otherwise; idle clips load while [0] < 0 */
static char sHeapArea[868352];

static inline void PurgeIdle(void)
{
    int i;

    FntPrint("PurgeIdle\n");
    for (i = 0; i < 1; i++) {
        if (sIdleSlots[i].state == CLIP_SLOT_READY && gPlayingIdleSlot != i) {
            sIdleSlots[i].state = CLIP_SLOT_FREE;
            FreeHeapBlock(sIdleSlots[i].buffer);
        }
    }
}

int gIdleClipReadPending;
int gSequenceClipPlaying;
int gIdleClipReadCount;

struct ClipFrameInfo gClipFrameInfo;

int gUnusedClipWord; /* 4 bytes; zeroed at init, nothing reads it */

void InitClipPlaybackSlots(void)
{
    int i;
    ClipSlot *slot;
    ClipSlot *fallback;
    FilePart *table;

    InitClipDecoder(320, 0);
    InitHeapArea(sHeapArea, sizeof(sHeapArea));
    LoadLoadingImage();
    gIdleClipReadPending = 0;
    gSequenceClipPlaying = 0;
    gClipPlaying = 0;
    gPlayingIdleSlot = 0;
    gNextSequenceSlot = 0;
    gIdleClipReadCount = 0;
    for (i = 0; i < 1; i++) {
        slot = &sIdleSlots[i];
        slot->unused = 0;
        slot->state = CLIP_SLOT_FREE;
        slot->playCount = 0;
    }
    for (i = 0; i < 6; i++) {
        sSequenceSlots[i].unused = 0;
        sSequenceSlots[i].state = CLIP_SLOT_FREE;
        sSequenceSlots[i].buffer = 0;
        sIdleLoadHolds[i] = -1;
    }
    table = gLapksParts;
    sDefaultIdleSlot.buffer = AllocHeapBlock(table[9].size);
    sIdleSlots[0].buffer = AllocHeapBlock(table[8].size);
    sDefaultIdleSlot.request = RequestFilePartRead(4, 9, table, sDefaultIdleSlot.buffer);
    fallback = &sDefaultIdleSlot;
    while (PollCdRequest(fallback->request) == 0) {
        DrawLoadingFrame(0);
    }
    fallback->state = CLIP_SLOT_READY;
    sIdleSlots[0].request = RequestFilePartRead(4, 8, gLapksParts, sIdleSlots[0].buffer);
    slot = sIdleSlots;
    while (PollCdRequest(slot->request) == 0) {
        DrawLoadingFrame(0);
    }
    slot->state = CLIP_SLOT_READY;
    gUnusedClipWord = 0;
}

void PollIdleClipReads(void)
{
    int i;
    int status;
    ClipSlot *slot;

    if (gIdleClipReadPending != 0) {
        for (i = 0; i < 1; i++) {
            slot = &sIdleSlots[i];
            if (slot->state == CLIP_SLOT_READING) {
                status = PollCdRequest(slot->request);
                if (status != 0) {
                    if (status > 0) {
                        slot->state = CLIP_SLOT_READY;
                        slot->playCount = 0;
                    } else {
                        FreeHeapBlock(slot->buffer);
                        slot->state = CLIP_SLOT_FAILED;
                    }
                    if (--gIdleClipReadCount == 0) {
                        gIdleClipReadPending = 0;
                    }
                }
            }
        }
    }
}

void LoadRandomIdleClip(int slotIndex)
{
    int n;

    if (gSequenceClipPlaying != 0 || gPlayingIdleSlot != slotIndex) {
        if (sIdleSlots[slotIndex].state == CLIP_SLOT_READY) {
            FreeHeapBlock(sIdleSlots[slotIndex].buffer);
        }
        n = rand() % 38;
        sIdleSlots[slotIndex].buffer = AllocHeapBlock(gLapksParts[gIdleClipParts[n]].size);
        if (sIdleSlots[slotIndex].buffer != 0) {
            sIdleSlots[slotIndex].request = RequestFilePartRead(4, gIdleClipParts[n], gLapksParts, sIdleSlots[slotIndex].buffer);
            sIdleSlots[slotIndex].state = CLIP_SLOT_READING;
            switch (gIdleClipParts[n]) {
            case 36:
                sIdleSlots[slotIndex].soundProgram = 20;
                break;
            case 42:
                sIdleSlots[slotIndex].soundProgram = 21;
                break;
            default:
                sIdleSlots[slotIndex].soundProgram = 0;
                break;
            }
            if (gIdleClipReadPending == 0) {
                gIdleClipReadPending = 1;
            }
            gIdleClipReadCount++;
        } else {
            sIdleSlots[slotIndex].state = CLIP_SLOT_FREE;
        }
    }
}

short gClipSoundProgram = 0;
int gClipPlaying;

void PlayIdleClip(void)
{
    int selected;
    int leastUsed;
    ClipSlot *cursor;
    int i;
    ClipSlot *slot;
    struct ClipFrameInfo *info;

    if (gClipPlaying == 0) {
        leastUsed = -1;
        selected = leastUsed;
        for (i = 0; i < 1; i++) {
            cursor = &sIdleSlots[i];
            if (cursor->state == CLIP_SLOT_READY &&
                (leastUsed < 0 || cursor->playCount < leastUsed)) {
                leastUsed = cursor->playCount;
                selected = i;
            }
        }
        if (selected != -1) {
            ClipSlot *start = sIdleSlots;
            slot = &start[selected];
            ++slot->playCount;
            OpenClip((unsigned char *)slot->buffer);
            if (slot->soundProgram != 0) {
                gClipSoundProgram = slot->soundProgram;
            }
        } else {
            OpenClip((unsigned char *)sDefaultIdleSlot.buffer);
        }
        gPlayingIdleSlot = selected;
        gClipPlaying = 1;
    }
    info = &gClipFrameInfo;
    DecodeClipFrame(info);
    if (info->soundCue != 0) {
        StartSound(gClipSoundProgram);
    }
    if (AdvanceClipFrame() == 0) {
        if (gClipSoundProgram != 0) {
            StopSound();
            gClipSoundProgram = 0;
        }
        if (gPlayingIdleSlot != -1) {
            sIdleSlots[gPlayingIdleSlot].state = CLIP_SLOT_FREE;
            FreeHeapBlock(sIdleSlots[gPlayingIdleSlot].buffer);
        }
        gClipPlaying = 0;
    }
}

int PlaySequenceClips(int count)
{
    ClipSlot *slot;

    if (gClipPlaying == 0) {
        if (gNextSequenceSlot < count) {
            ClipSlot *slots = sSequenceSlots;
            OpenClip((unsigned char *)slots[gNextSequenceSlot].buffer);
            if (slots[gNextSequenceSlot].soundProgram != 0) {
                gClipSoundProgram = slots[gNextSequenceSlot].soundProgram;
            }
            gNextSequenceSlot++;
            gSequenceClipPlaying = 1;
            gClipPlaying = 1;
        } else {
            PlayIdleClip();
            return 0;
        }
    }
    DecodeClipFrame(&gClipFrameInfo);
    if (AdvanceClipFrame() == 0) {
        if (gClipSoundProgram != 0) {
            gClipSoundProgram = 0;
        }
        if (gSequenceClipPlaying == 1) {
            ClipSlot *slots = sSequenceSlots;
            int index = gNextSequenceSlot - 1;
            slot = slots + index;
            slot->state = CLIP_SLOT_FREE;
            FreeHeapBlock(slot->buffer);
            gSequenceClipPlaying = 0;
            index = gNextSequenceSlot - 1;
            (slots + index)->buffer = 0;
        }
        gClipPlaying = 0;
        if (gNextSequenceSlot == count) {
            return 0;
        }
    }
    if (gSequenceClipPlaying != 0) {
        return 1;
    }
    return -1;
}

int LoadColumnTurnClips(int group)
{
    int i;
    int *entries;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gColumnTurnClipParts[group][0]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[1].buffer = AllocHeapBlock(gLapksParts[gColumnTurnClipParts[group][1]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[2].buffer = AllocHeapBlock(gLapksParts[gColumnTurnClipParts[group][2]].size)) == 0) {
        PurgeIdle();
    }
    if (sSequenceSlots[0].buffer != 0 && sSequenceSlots[1].buffer != 0 && sSequenceSlots[2].buffer != 0) {
        entries = gColumnTurnClipParts[group];
        for (i = 0; i < 3; i++) {
            sSequenceSlots[i].request = RequestFilePartRead(4, entries[i], gLapksParts, sSequenceSlots[i].buffer);
            sSequenceSlots[i].state = CLIP_SLOT_READING;
            sSequenceSlots[i].soundProgram = gColumnTurnSoundPrograms[i].program;
        }
        gNextSequenceSlot = 0;
        return 1;
    }
    FreeHeapBlock(sSequenceSlots[0].buffer);
    FreeHeapBlock(sSequenceSlots[1].buffer);
    FreeHeapBlock(sSequenceSlots[2].buffer);
    return 0;
}

int gLevelMoveClipParts[2][1] = { 14, 15 };
ClipSoundProgram gLevelMoveSoundPrograms[1] = {
    { 0, 0 },
};

int LoadLevelMoveClip(int group)
{
    int i;
    int *entries;
    ClipSoundProgram *soundPrograms;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gLevelMoveClipParts[group][0]].size)) != 0 ||
        (PurgeIdle(), (sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gLevelMoveClipParts[group][0]].size)) != 0)) {
        soundPrograms = gLevelMoveSoundPrograms;
        entries = gLevelMoveClipParts[group];
        for (i = 0; i < 1; i++) {
            sSequenceSlots[i].request = RequestFilePartRead(4, entries[i], gLapksParts, sSequenceSlots[i].buffer);
            sSequenceSlots[i].state = CLIP_SLOT_READING;
            sSequenceSlots[i].soundProgram = soundPrograms[i].program;
        }
    } else {
        sSequenceSlots[0].state = CLIP_SLOT_FAILED;
        return 0;
    }
    gNextSequenceSlot = 0;
    return 1;
}

int LoadNodeOpenClips(int clipSet)
{
    int i;
    int k;
    int *row;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gNodeOpenClipParts[clipSet][0]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[1].buffer = AllocHeapBlock(gLapksParts[gNodeOpenClipParts[clipSet][1]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[2].buffer = AllocHeapBlock(gLapksParts[gNodeOpenClipParts[clipSet][2]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[3].buffer = AllocHeapBlock(gLapksParts[gNodeOpenClipParts[clipSet][3]].size)) == 0) {
        PurgeIdle();
    }
    if (sSequenceSlots[0].buffer != 0 && sSequenceSlots[1].buffer != 0 && sSequenceSlots[2].buffer != 0 &&
        sSequenceSlots[3].buffer != 0) {
        row = gNodeOpenClipParts[clipSet];
        for (k = 0; k < 4; k++) {
            sSequenceSlots[k].request = RequestFilePartRead(4, row[k], gLapksParts, sSequenceSlots[k].buffer);
            sSequenceSlots[k].state = CLIP_SLOT_READING;
            sSequenceSlots[k].soundProgram = gNodeOpenSoundPrograms[k].program;
        }
        gNextSequenceSlot = 0;
        gLoadedOpenClipSet = clipSet;
        return 1;
    }
    for (i = 0; i < 4; i++) {
        if (sSequenceSlots[i].buffer != 0) {
            FreeHeapBlock(sSequenceSlots[i].buffer);
        }
    }
    return 0;
}

int gSystemMenuClipParts[1] = { 52 };
ClipSoundProgram gSystemMenuSoundPrograms[1] = {
    { 0, 0 },
};

int LoadSystemMenuClip(void)
{
    int i;
    int *entries;
    ClipSoundProgram *soundPrograms;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gSystemMenuClipParts[0]].size)) != 0 ||
        (PurgeIdle(), (sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gSystemMenuClipParts[0]].size)) != 0)) {
        soundPrograms = gSystemMenuSoundPrograms;
        entries = gSystemMenuClipParts;
        for (i = 0; i < 1; i++) {
            sSequenceSlots[i].request = RequestFilePartRead(4, entries[i], gLapksParts, sSequenceSlots[i].buffer);
            sSequenceSlots[i].state = CLIP_SLOT_READING;
            sSequenceSlots[i].soundProgram = soundPrograms[i].program;
        }
    } else {
        sSequenceSlots[0].state = CLIP_SLOT_FAILED;
        return 0;
    }
    gNextSequenceSlot = 0;
    return 1;
}

int gGridEntryClipParts[1] = { 54 };
ClipSoundProgram gGridEntrySoundPrograms[1] = {
    { 0, 0 },
};

int LoadGridEntryClip(void)
{
    int i;
    int *entries;
    ClipSoundProgram *soundPrograms;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gGridEntryClipParts[0]].size)) != 0 ||
        (PurgeIdle(), (sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gGridEntryClipParts[0]].size)) != 0)) {
        soundPrograms = gGridEntrySoundPrograms;
        entries = gGridEntryClipParts;
        for (i = 0; i < 1; i++) {
            sSequenceSlots[i].request = RequestFilePartRead(4, entries[i], gLapksParts, sSequenceSlots[i].buffer);
            sSequenceSlots[i].state = CLIP_SLOT_READING;
            sSequenceSlots[i].soundProgram = soundPrograms[i].program;
        }
    } else {
        sSequenceSlots[0].state = CLIP_SLOT_FAILED;
        return 0;
    }
    gNextSequenceSlot = 0;
    return 1;
}

int LoadVoiceScreenClips(void)
{
    int i;
    int k;
    int *parts;

    if ((sSequenceSlots[0].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[0]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[1].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[1]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[2].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[2]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[3].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[3]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[4].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[4]].size)) == 0) {
        PurgeIdle();
    }
    if ((sSequenceSlots[5].buffer = AllocHeapBlock(gLapksParts[gVoiceScreenClipParts[5]].size)) == 0) {
        PurgeIdle();
    }
    if (sSequenceSlots[0].buffer != 0 && sSequenceSlots[1].buffer != 0 && sSequenceSlots[2].buffer != 0 &&
        sSequenceSlots[3].buffer != 0 && sSequenceSlots[4].buffer != 0 && sSequenceSlots[5].buffer != 0) {
        parts = gVoiceScreenClipParts;
        for (k = 0; k < 6; k++) {
            sSequenceSlots[k].request = RequestFilePartRead(4, parts[k], gLapksParts, sSequenceSlots[k].buffer);
            sSequenceSlots[k].state = CLIP_SLOT_READING;
            sSequenceSlots[k].soundProgram = gVoiceScreenSoundPrograms[k].program;
        }
        gNextSequenceSlot = 0;
        return 1;
    }
    for (i = 0; i < 6; i++) {
        if (sSequenceSlots[i].buffer != 0) {
            FreeHeapBlock(sSequenceSlots[i].buffer);
        }
    }
    return 0;
}

int PlayLastVoiceScreenClip(void)
{
    int i;
    int index;
    void *clipBuffer;
    ClipSlot *slot;

    if (gClipPlaying == 0 && gNextSequenceSlot == 5) {
        OpenClip((unsigned char *)sSequenceSlots[5].buffer);
        gSequenceClipPlaying = 1;
        gClipPlaying = 1;
        gNextSequenceSlot++;
        for (i = 0; i < 5; i++) {
            sSequenceSlots[i].state = CLIP_SLOT_FREE;
            FreeHeapBlock(sSequenceSlots[i].buffer);
            sSequenceSlots[i].buffer = 0;
        }
    }
    DecodeClipFrame(&gClipFrameInfo);
    if (AdvanceClipFrame() == 0) {
        if (gClipSoundProgram != 0) {
            gClipSoundProgram = 0;
        }
        if (gSequenceClipPlaying == 1) {
            ClipSlot *cleanupSlots = sSequenceSlots;
            index = gNextSequenceSlot - 1;
            slot = cleanupSlots + index;
            clipBuffer = slot->buffer;
            slot->state = CLIP_SLOT_FREE;
            FreeHeapBlock(clipBuffer);
            gSequenceClipPlaying = 0;
            index = gNextSequenceSlot - 1;
            cleanupSlots[index].buffer = 0;
        }
        gClipPlaying = 0;
        if (gNextSequenceSlot == 6) {
            return 0;
        }
    }
    if (gSequenceClipPlaying != 0) {
        return 1;
    }
    return -1;
}

int UnusedReturnOne(void)
{
    return 1;
}

void UnusedClipStub1(void)
{
}

void UnusedClipStub2(void)
{
}

void UnusedClipStub3(void)
{
}

void UpdateIdleClips(int skipLoad)
{
    int slotIndex;
    int i;

    PollIdleClipReads();
    if (!skipLoad && sIdleLoadHolds[0] < 0) {
        slotIndex = -1;
        for (i = 0; i < 1; i++) {
            if (sIdleSlots[i].state == CLIP_SLOT_FREE) {
                slotIndex = i;
                break;
            }
        }
        if (slotIndex != -1) {
            LoadRandomIdleClip(slotIndex);
        }
    }
    PlayIdleClip();
}

int PollColumnTurnClips(void)
{
    int i;
    ClipSlot *slot;

    for (i = 0; i < 3; i++) {
        slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
    }
    return 1;
}

int PlayColumnTurnClips(void)
{
    return PlaySequenceClips(3);
}

int PollLevelMoveClip(void)
{
    int i;
    ClipSlot *slot;

    for (i = 0; i <= 0; i++) {
        slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
    }
    return 1;
}

int PlayLevelMoveClip(void)
{
    return PlaySequenceClips(1);
}

int PollNodeOpenClips(void)
{
    int i;
    ClipSlot *slot;

    for (i = 0; i < 4; i++) {
        slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
    }
    return 1;
}

int PlayNodeOpenClips(void)
{
    return PlaySequenceClips(4);
}

int PollSystemMenuClip(void)
{
    int i;
    ClipSlot *slot;

    for (i = 0; i <= 0; i++) {
        slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
    }
    return 1;
}

int PlaySystemMenuClip(void)
{
    return PlaySequenceClips(1);
}

int PollGridEntryClip(void)
{
    int i;
    ClipSlot *slot;

    for (i = 0; i <= 0; i++) {
        slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
    }
    return 1;
}

int PlayGridEntryClip(void)
{
    return PlaySequenceClips(1);
}

short gFirstFrameSlots[4] = { 3, 1, 2, 4 };

void FreeIdleClips(void)
{
    int i;
    ClipSlot *slot;

    if (gIdleClipReadPending == 0) {
        gClipPlaying = 0;
        for (i = 0; i <= 0; i++) {
            slot = &sIdleSlots[i];
            if (slot->state == CLIP_SLOT_READY) {
                slot->state = CLIP_SLOT_FREE;
                FreeHeapBlock(slot->buffer);
            }
        }
    }
}

int PollVoiceScreenClips(void)
{
    int i = 0;

    do {
        ClipSlot *slot = &sSequenceSlots[i];
        if (slot->state != CLIP_SLOT_READY) {
            if (slot->state == CLIP_SLOT_FAILED) {
                FntPrint("MEMORY TOO FEW\n");
                return 0;
            }
            if (PollCdRequest(slot->request) == 0) {
                return 0;
            }
            slot->state = CLIP_SLOT_READY;
        }
        i++;
    } while (i < 6);
    return 1;
}

int PlayFirstVoiceScreenClip(void)
{
    if (gClipPlaying == 0) {
        ClipSlot *slots = sSequenceSlots;
        OpenClip((unsigned char *)slots[gNextSequenceSlot].buffer);
        if (slots[gNextSequenceSlot].soundProgram != 0) {
            gClipSoundProgram = slots[gNextSequenceSlot].soundProgram;
        }
        gNextSequenceSlot++;
        gSequenceClipPlaying = 1;
        gClipPlaying = 1;
    }
    DecodeClipFrame(&gClipFrameInfo);
    if (AdvanceClipFrame() == 0) {
        if (gClipSoundProgram != 0) {
            gClipSoundProgram = 0;
        }
        gClipPlaying = 0;
        gNextSequenceSlot = 5;
        gClipPlaying = 0;
        return 0;
    }
    if (gSequenceClipPlaying != 0) {
        return 1;
    }
    return -1;
}

void DecodeSequenceClipFirstFrame(int index)
{
    int slot;
    ClipSlot *slots;

    if (index < 0) {
        index = 0;
    }
    slots = sSequenceSlots;
    slot = gFirstFrameSlots[index];
    OpenClip((unsigned char *)slots[slot].buffer);
    DecodeClipFrame(&gClipFrameInfo);
}

int AreIdleClipReadsDone(void)
{
    return (unsigned char)(gIdleClipReadPending == 0);
}

short gIdleClipParts[38] = {
    8, 16, 8, 34, 8, 35, 8, 36, 8, 37, 8, 38, 8, 43, 8, 45, 8, 46, 8, 47, 8, 48, 8, 49,
    8, 50, 8, 33, 8, 39, 8, 40, 8, 41, 8, 42, 8, 44,
};
int gColumnTurnClipParts[2][3] = {
    { 0, 18, 1 },
    { 2, 20, 3 },
};
ClipSoundProgram gColumnTurnSoundPrograms[3] = {
    { 0, 0 },
    { 11, 0 },
    { 0, 0 },
};
int gNodeOpenClipParts[5][4] = {
    { 24, 27, 29, 25 },
    { 24, 27, 30, 25 },
    { 21, 4, 22, 9 },
    { 24, 31, 32, 25 },
    { 24, 31, 5, 25 },
};
ClipSoundProgram gNodeOpenSoundPrograms[4] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};
int gVoiceScreenClipParts[6] = { 53, 55, 56, 58, 7, 57 };
ClipSoundProgram gVoiceScreenSoundPrograms[6] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};

GridCell gGridCells[23][24];
