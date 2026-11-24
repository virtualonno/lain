#ifndef LAPKS_PLAYER_H
#define LAPKS_PLAYER_H

#include "clip.h"
#include "grid_nodes.h"

typedef struct {
    short program;
    short unused; /* nothing reads it */
} ClipSoundProgram;

extern struct ClipFrameInfo gClipFrameInfo;

enum NodeOpenClipSet {
    OPEN_CLIP_SPIN_AWAY,
    OPEN_CLIP_COLLAPSE_SPLIT,
    OPEN_CLIP_REFUSE_SPARK,
    OPEN_CLIP_REFUSE_RETURN,
    OPEN_CLIP_REFUSE_VANISH
};
extern short gLoadedOpenClipSet;
extern int gClipPlaying;
extern int gSystemMenuClipParts[1];
extern ClipSoundProgram gSystemMenuSoundPrograms[1];
extern short gIdleClipParts[38];
extern int gColumnTurnClipParts[2][3];
extern ClipSoundProgram gColumnTurnSoundPrograms[3];
extern int gNodeOpenClipParts[5][4];
extern ClipSoundProgram gNodeOpenSoundPrograms[4];
extern int gVoiceScreenClipParts[6];
extern ClipSoundProgram gVoiceScreenSoundPrograms[6];

void InitClipPlaybackSlots(void);
int PlaySequenceClips(int count);
int LoadColumnTurnClips(int group);
int LoadLevelMoveClip(int group);
int LoadNodeOpenClips(int clipSet);
int LoadSystemMenuClip(void);
int LoadGridEntryClip(void);
int LoadVoiceScreenClips(void);
int PlayLastVoiceScreenClip(void);
void UpdateIdleClips(int skipLoad);
int PollColumnTurnClips(void);
int PlayColumnTurnClips(void);
int PollLevelMoveClip(void);
int PlayLevelMoveClip(void);
int PollNodeOpenClips(void);
int PlayNodeOpenClips(void);
int PollSystemMenuClip(void);
int PlaySystemMenuClip(void);
int PollGridEntryClip(void);
int PlayGridEntryClip(void);
void FreeIdleClips(void);
int PollVoiceScreenClips(void);
int PlayFirstVoiceScreenClip(void);
int AreIdleClipReadsDone(void);

extern GridCell gGridCells[23][24];

#endif
