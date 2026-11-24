/* Speech from romaji: joins the kana WAV clips of VOICE.BIN (file 7) named by the lines of
 * BIN.BIN part 20, trims each join, and clamps the 16-bit PCM to a piecewise-linear envelope. */
#include <sys/types.h>
#include <memory.h>
#include <strings.h>
#include <stdio.h>
#include "file_parts.h"
#include "cd.h"
#include "heap.h"
#include "loading.h"
#include "media_player.h"
#include "napk.h"
#include "speech.h"

/* The lines of gVoicePartNames searched for a WAV name. */
#define VOICE_NAME_COUNT 638

/* One WAV clip read from VOICE.BIN (file 7). headTrim and tailTrim are 1 or 0 until
   TrimSpeechClips turns them into byte counts cut from each end of length. */
typedef struct {
    void *data;
    int headTrim;
    int tailTrim;
    short hasUnderscore;
    int length;
} SpeechClip;

int gSpeechPcmSize = 0;
short gLoadedClipCount = 0;
char *gVoicePartNames;
static SpeechClip sSpeechClips[16];
static char sUnusedBytes[32]; /* 32 bytes; nothing reads them */

int InterpolateCurve(unsigned int value, CurvePoint *points, unsigned int count)
{
    unsigned int i;
    float t;
    int result;

    if (count < 2) {
        return 0;
    }
    if ((float)value <= points[0].x) {
        return points[0].y;
    }
    if ((float)value >= points[count - 1].x) {
        return points[count - 1].y;
    }
    for (i = 0; i < count; i++) {
        if ((float)value < points[i].x) {
            t = ((float)value - points[i - 1].x) / (points[i].x - points[i - 1].x);
            result = (1.0f - t) * points[i - 1].y + t * points[i].y;
            break;
        }
    }
    return result;
}

void LimitSpeechAmplitude(short *samples)
{
    int i;
    int limit;
    int sample;
    int clipped;
    CurvePoint *curve = gSpeechEnvelope;

    gSpeechEnvelope[0].x = 0.0f;
    curve[1].x = gSpeechPcmSize / 4;
    curve[2].x = curve[1].x + curve[1].x;
    i = gSpeechEnvelope[0].x;
    while ((float)i < curve[2].x) {
        limit = InterpolateCurve(i, curve, 3);
        sample = *samples;
        if (sample >= 0) {
            clipped = limit < sample;
        } else {
            limit = -limit;
            clipped = sample < limit;
        }
        if ((signed char)clipped) {
            *samples = limit;
        }
        samples++;
        i++;
    }
}

int LoadSpeechClip(char *name)
{
    short i;
    short hasUnderscore;
    int headTrim;
    unsigned short tailTrim;
    int j;
    int part;
    void *data;
    int len;
    char *line;
    int request;
    FilePart *entries;
    FilePart *entry;
    char buf[16];

    i = 0;
    hasUnderscore = 0;
    for (; name[i] != 0; i++) {
        if (name[i] == '_') {
            hasUnderscore = 1;
            break;
        }
    }
    if (gLoadedClipCount == 0) {
        headTrim = 0;
        tailTrim = 1;
    } else {
        headTrim = 1;
        tailTrim = 1;
        if (!hasUnderscore) {
            headTrim = 0;
            if (gLoadedClipCount > 0) {
                sSpeechClips[gLoadedClipCount - 1].tailTrim = 0;
            }
        }
    }

    entries = gVoiceParts;
    for (j = 0, line = gVoicePartNames; j < VOICE_NAME_COUNT; j++) {
        len = 0;
        while (*line != '\r') {
            buf[len++] = *line++;
        }
        buf[len] = 0;
        if (strncmp(name, buf, len) == 0) {
            break;
        }
        line += 2;
    }
    entry = &entries[j];
    data = AllocHeapBlock(entry->size);
    sSpeechClips[gLoadedClipCount].data = data;

    line = gVoicePartNames;
    for (part = 0; part < VOICE_NAME_COUNT; part++) {
        len = 0;
        while (*line != '\r') {
            buf[len++] = *line++;
        }
        buf[len] = 0;
        if (strncmp(name, buf, len) == 0) {
            break;
        }
        line += 2;
    }
    request = RequestFilePartRead(7, part, gVoiceParts, data);
    while (PollCdRequest(request) == 0) {
        DrawSpeechLoadingFrame();
    }

    sSpeechClips[gLoadedClipCount].headTrim = headTrim;
    sSpeechClips[gLoadedClipCount].tailTrim = tailTrim;
    sSpeechClips[gLoadedClipCount].hasUnderscore = hasUnderscore;
    line = gVoicePartNames;
    for (j = 0; j < VOICE_NAME_COUNT; j++) {
        len = 0;
        while (*line != '\r') {
            buf[len++] = *line++;
        }
        buf[len] = 0;
        if (strncmp(name, buf, len) == 0) {
            break;
        }
        line += 2;
    }
    sSpeechClips[gLoadedClipCount].length = gVoiceParts[j].size;
    gLoadedClipCount++;
    return 1;
}

typedef struct {
    u_char symbols[7];
} SingleKana;

extern SingleKana gSingleKanaSymbols[];
u_char gPreviousPhoneticSymbol;
short *gSpeechPcm;

static __inline__ int HasVoicePart(char *name, char *entryBuf, char *entry)
{
    unsigned int row;
    int len;
    char *line;

    line = gVoicePartNames;
    for (row = 0; row < VOICE_NAME_COUNT; row++, line += 2) {
        len = 0;
        while ((u_char)*line != '\r') {
            entryBuf[len++] = *line++;
        }
        entryBuf[len] = '\0';
        if (strncmp(name, entry, len) == 0) {
            return 1;
        }
    }
    return 0;
}

short *BuildSpeechPcm(u_char *romaji, int *pcmByteCount, u_char *clipCount)
{
    SingleKana singleKana = gSingleKanaSymbols[0];
    char wavName[16];
    u_char syllable[4];
    char nameLine[16];
    char *nameBuffer;
    char *format;
    void *packedNames;
    int request;
    short i;
    short matchedSingle;

    gPreviousPhoneticSymbol = '\\';
    gSpeechPcmSize = 0;
    gLoadedClipCount = 0;
    packedNames = AllocHeapBlock(gBinParts[20].size);
    request = RequestFilePartRead(3, 20, gBinParts, packedNames);
    while (PollCdRequest(request) == 0) {
        DrawSpeechLoadingFrame();
    }
    gVoicePartNames = (char *)UnpackNapk(packedNames, gBinParts[20].size);
    FreeHeapBlock(packedNames);

    while (*romaji != '\0') {
        matchedSingle = 0;
        for (i = 0; i < 7; i++) {
            u_char symbol = singleKana.symbols[i];
            nameBuffer = nameLine;
            if (*romaji == symbol) {
                if (*romaji != '-') {
                    if (gPreviousPhoneticSymbol != '\\') {
                        format = "%c_%c.WAV";
                        sprintf(wavName, format, gPreviousPhoneticSymbol, singleKana.symbols[i]);
                        if (!HasVoicePart(wavName, nameBuffer, nameLine)) {
                            format = gSingleKanaWavFormat;
                            sprintf(wavName, format, singleKana.symbols[i]);
                        }
                    } else {
                        format = gSingleKanaWavFormat;
                        sprintf(wavName, format, singleKana.symbols[i]);
                    }
                } else {
                    format = "%c_%c.WAV";
                    sprintf(wavName, format, gPreviousPhoneticSymbol, gPreviousPhoneticSymbol);
                }
                LoadSpeechClip(wavName);
                matchedSingle = 1;
                gPreviousPhoneticSymbol = singleKana.symbols[i];
                romaji++;
                break;
            }
        }
        if (!matchedSingle) {
            syllable[0] = *romaji++;
            syllable[1] = *romaji++;
            syllable[2] = '\0';
            if (syllable[1] == 'Y' || syllable[1] == 'H') {
                syllable[2] = *romaji++;
                syllable[3] = '\0';
            }
            if (gPreviousPhoneticSymbol != '\\') {
                sprintf(wavName, "%c_%s.WAV", gPreviousPhoneticSymbol, syllable);
                if (!HasVoicePart(wavName, nameBuffer, nameLine)) {
                    sprintf(wavName, gSyllableWavFormat, syllable);
                }
            } else {
                sprintf(wavName, gSyllableWavFormat, syllable);
            }
            if (syllable[1] == 'Y' || syllable[1] == 'H') {
                gPreviousPhoneticSymbol = syllable[2];
            } else {
                gPreviousPhoneticSymbol = syllable[1];
            }
            LoadSpeechClip(wavName);
        }
    }

    if (gLoadedClipCount > 0) {
        sSpeechClips[gLoadedClipCount - 1].tailTrim = 0;
    }
    TrimSpeechClips();
    gSpeechPcm = (short *)AllocHeapBlock(gSpeechPcmSize);
    JoinSpeechClips((char *)gSpeechPcm);
    LimitSpeechAmplitude(gSpeechPcm);
    *pcmByteCount = gSpeechPcmSize;
    *clipCount = gLoadedClipCount;
    FreeHeapBlock(gVoicePartNames);
    return gSpeechPcm;
}

SingleKana gSingleKanaSymbols[1] = { { "AIUEO#-" } };
char gSingleKanaWavFormat[] = "%c.WAV";
char gSyllableWavFormat[] = "%s.WAV";

void TrimSpeechClips(void)
{
    int i;

    gSpeechPcmSize = 0;
    for (i = 0; i < gLoadedClipCount; i++) {
        SpeechClip *clip = &sSpeechClips[i];
        int length = clip->length;
        unsigned int trim = (10 * (unsigned int)length) / 45;

        if (trim % 2)
            trim = 2 * (trim / 2 + 1);

        if (clip->headTrim == 1) {
            clip->length = length - (trim << 1);
            clip->headTrim = trim << 1;
        }
        if (clip->tailTrim == 1) {
            clip->tailTrim = trim;
            clip->length -= trim;
        }
        gSpeechPcmSize += clip->length;
    }
}

int FindVoicePart(char *name)
{
    char *line = gVoicePartNames;
    int index;
    char entry[16];
    int length;

    for (index = 0; index < VOICE_NAME_COUNT; index++, line += 2) {
        length = 0;
        while ((u_char)*line != '\r') {
            entry[length++] = *line++;
        }
        entry[length] = '\0';
        if (strncmp(name, entry, length) == 0) {
            break;
        }
    }
    return index;
}

void LoadVoicePartNames(void)
{
    void *buffer;
    int request;

    buffer = AllocHeapBlock(gBinParts[20].size);
    request = RequestFilePartRead(3, 20, gBinParts, buffer);
    while (PollCdRequest(request) == 0) {
        DrawSpeechLoadingFrame();
    }
    gVoicePartNames = (char *)UnpackNapk(buffer, gBinParts[20].size);
    FreeHeapBlock(buffer);
}

int ReadVoicePart(char *name, void *dest)
{
    char *line = gVoicePartNames;
    int index;
    char entry[16];
    int length;
    int request;
    int done;

    for (index = 0; index < VOICE_NAME_COUNT; index++, line += 2) {
        length = 0;
        while ((u_char)*line != '\r') {
            entry[length++] = *line++;
        }
        entry[length] = '\0';
        if (strncmp(name, entry, length) == 0) {
            break;
        }
    }
    request = RequestFilePartRead(7, index, gVoiceParts, dest);
    while ((done = PollCdRequest(request)) == 0) {
        DrawSpeechLoadingFrame();
    }
    return done;
}

void JoinSpeechClips(char *dest)
{
    int i;

    for (i = 0; i < gLoadedClipCount; i++) {
        memcpy(dest, (char *)sSpeechClips[i].data + sSpeechClips[i].headTrim, sSpeechClips[i].length);
        dest += sSpeechClips[i].length;
        FreeHeapBlock(sSpeechClips[i].data);
    }
}

void DrawSpeechLoadingFrame(void)
{
    if (gTrackIndex != 733) {
        DrawLoadingFrame(0);
    }
}

CurvePoint gSpeechEnvelope[3] = {
    { 0.0f, 5000.0f },
    { 0.0f, 15000.0f },
    { 0.0f, 5000.0f },
};
