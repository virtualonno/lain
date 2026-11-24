#ifndef CLIP_H
#define CLIP_H

struct ClipFrameInfo {
    int width;
    int height;
    int x;
    int y;
    int soundCue;
};

/* Incremented by CountVSync from the VSyncCallback handler; src/media_player.c declares it as a plain int. */
extern volatile int gVSyncCount;

int DecodeClipFrame(struct ClipFrameInfo *info);
void InitClipDecoder(int x, int y);
int OpenClip(unsigned char *data);
int AdvanceClipFrame(void);
int CountVSync(void);

#endif
