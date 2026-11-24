#ifndef SPEECH_H
#define SPEECH_H

#include <sys/types.h>

typedef struct {
    float x;
    float y;
} CurvePoint;

extern char gSingleKanaWavFormat[];
extern char gSyllableWavFormat[];
extern CurvePoint gSpeechEnvelope[3];

int InterpolateCurve(unsigned int value, CurvePoint *points, unsigned int count);
short *BuildSpeechPcm(u_char *romaji, int *pcmByteCount, u_char *clipCount);
void TrimSpeechClips(void);
void JoinSpeechClips(char *dest);
void DrawSpeechLoadingFrame(void);

#endif
