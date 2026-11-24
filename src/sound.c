/* LIBSND sound: loads SND.BIN's VAB (parts 0 and 1) and its grid and About screen SEQs (parts 2
 * and 3), keys VAB programs on and off, and plays, stops and fades the selected SEQ. LIBSND gets
 * voices 0..22; voice_screen.c plays on voice 23. */
#include <sys/types.h>
#include <libsnd.h>
#include "loading.h"
#include "cd.h"
#include "heap.h"
#include "file_parts.h"
#include "sound.h"

static u_char sVabHeader[18976];
static u_long sGridMusicData[768];
static u_long sAboutMusicData[768];
static char sSeqTable[SS_SEQ_TABSIZ * 2];
short gVabId;
short gMusicIndex;
short gGridMusicSeq;
short gAboutMusicSeq;
u_char gUnusedSoundBuf[16] = { 0 }; /* 16 bytes; PlaySound writes element 0, nothing reads it */

void InitSound(int reset)
{
    int request;
    u_char *body;
    int i;

    if (reset == 0) {
        SsInit();
        SsSetMVol(0, 0);
        SsSetReservedVoice(23);
        SsSetTableSize(sSeqTable, 2, 1);
        SsSetTickMode(SS_TICK240);
        request = RequestFilePartRead(5, 0, gSndParts, sVabHeader);
        while (PollCdRequest(request) == 0) {
            DrawLoadingFrame(0);
        }
        body = AllocHeapBlock(gSndParts[1].size);
        request = RequestFilePartRead(5, 1, gSndParts, body);
        while (PollCdRequest(request) == 0) {
            DrawLoadingFrame(0);
        }
        request = RequestFilePartRead(5, 2, gSndParts, sGridMusicData);
        while (PollCdRequest(request) == 0) {
            DrawLoadingFrame(0);
        }
        request = RequestFilePartRead(5, 3, gSndParts, sAboutMusicData);
        while (PollCdRequest(request) == 0) {
            DrawLoadingFrame(0);
        }
        gVabId = SsVabOpenHead(sVabHeader, -1);
        if (gVabId != -1) {
            if (SsVabTransBody(body, gVabId) == gVabId) {
                while (SsVabTransCompleted(SS_IMEDIATE) == 0) {
                    DrawLoadingFrame(0);
                }
                FreeHeapBlock(body);
                SsUtSetReverbType(SS_REV_TYPE_ROOM);
                SsUtReverbOn();
                SsUtSetReverbDepth(48, 48);
                gGridMusicSeq = SsSeqOpen(sGridMusicData, gVabId);
                gAboutMusicSeq = SsSeqOpen(sAboutMusicData, gVabId);
                gMusicIndex = 0;
                if (reset == 0) {
                    SsStart();
                    for (i = 0; i < 10; i++) {
                        DrawLoadingFrame(0);
                    }
                    SsSetMVol(127, 127);
                }
            }
        }
    }
}

void KeyOffVoice0(void)
{
    SsUtKeyOff(0, 0, 0, 0, 0x40);
}

void KeyOnProgram24(void)
{
    SsUtKeyOn(gVabId, 24, 0, 60, 0, 127, 127);
}

void SoundVSync(void)
{
}

short gStartedSoundVoice;

short gSoundVoice;

void PlaySound(int program)
{
    gUnusedSoundBuf[0] = 120;
    gSoundVoice = SsUtKeyOn(gVabId, (short)program, 0, 60, 0, 127, 127);
}

void StartSound(int program)
{
    gStartedSoundVoice = SsUtKeyOn(gVabId, (short)program, 0, 60, 0, 127, 127);
}

void StopSound(void)
{
    SsUtKeyOffV(gStartedSoundVoice);
}

void PlayMusic(void)
{
    int seq;
    int mode;

    if (gMusicIndex == 0) {
        SsSeqSetVol(gGridMusicSeq, 127, 127);
        seq = gGridMusicSeq;
        mode = SSPLAY_PLAY;
    } else {
        SsSeqSetVol(gAboutMusicSeq, 127, 127);
        mode = SSPLAY_PLAY;
        seq = gAboutMusicSeq;
    }
    SsSeqPlay(seq, mode, SSPLAY_INFINITY);
}

void StopMusic(void)
{
    int seq;

    if (gMusicIndex == 0) {
        seq = gGridMusicSeq;
    } else {
        seq = gAboutMusicSeq;
    }
    SsSeqStop(seq);
}

void FadeOutMusic(void)
{
    if (gMusicIndex == 0) {
        SsSeqSetDecrescendo(gGridMusicSeq, 127, 240);
    } else {
        SsSeqSetDecrescendo(gAboutMusicSeq, 127, 240);
    }
}

void FadeInMusic(void)
{
    if (gMusicIndex == 0) {
        SsSeqSetCrescendo(gGridMusicSeq, 127, 240);
    } else {
        SsSeqSetCrescendo(gAboutMusicSeq, 127, 240);
    }
}

void SelectMusic(int index)
{
    gMusicIndex = index;
}

void StopGridMusic(void)
{
    SsSeqStop(gGridMusicSeq);
}
