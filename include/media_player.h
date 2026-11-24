#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libcd.h>

enum MediaPlayerResult {
    MEDIA_RESULT_NONE,
    MEDIA_RESULT_END_ROLL,
    MEDIA_RESULT_COMPLETE
};

enum PlayerSelection {
    PLAYER_SELECT_EXIT,
    PLAYER_SELECT_PLAY,
    PLAYER_SELECT_WORD,
    PLAYER_SELECT_WORD_UP,
    PLAYER_SELECT_WORD_DOWN
};

enum EndRollState {
    END_ROLL_NONE,
    END_ROLL_READY,
    END_ROLL_PENDING,
    END_ROLL_PLAYING
};

/* A playable track. xaFile is 0 for a movie, else a 1-based row of gXaFiles;
   source is the movie's row of gDiscFiles or the track's XA channel; length is
   the movie's frame count or the XA channel's size in bytes; unused is 0 in every
   track and nothing reads it. */
typedef struct {
    u_char xaFile;
    u_char unused;
    short source;
    int length;
} Track;

extern Track gTracks[736];

/* A file on the disc: its first sector and its size in bytes. */
typedef struct {
    int sector;
    int size;
} DiscFile;

typedef struct {
    u_long *runLevel[2];
    int runLevelIndex;
    u_long *slice[2];
    int sliceIndex;
    RECT frameRect[2];
    int frameIndex;
    RECT sliceRect;
    int frameDone;
    u_char unused[8];
} MovieDecoder;

/* A movie frame as the stream ring hands it over: its MDEC quantizers, its number and its
   NITR-BS bitstream, which opens with its run/level count. */
typedef struct {
    u_char lumaQuant;
    u_char chromaQuant;
    short number;
    u_long bitstream[1];
} MovieFrame;

typedef struct {
    u_char showStill;
    short start;
    short end;
} EndRollSegment;

extern int gTrackIndex;
extern unsigned char gTrackXaFile;
extern u_char gTrackImageScaleDisabled;
extern unsigned char gEndRollState;
extern short gDecodedDataFlag;
extern void *gDecodedAudio;
extern short gTrackDone;
extern int gSpuIrqAddr;
extern u_char gCompletedFrameIndex;
extern int gXaStartSector;
extern int gXaSector;
extern int gXaEndSector;
extern CdlLOC gMovieSeekLoc;
extern DiscFile gDiscFiles[100];
extern DiscFile gDisc1Files[69];
extern DiscFile gDisc2Files[57];
extern int gXaFiles[21];
extern u_char gWordRing[30][2];
extern short gIdleTrackImageParts[2][10][3];
extern EndRollSegment gEndRollSegments[57];
extern GsLINE gFocusLines[2];
extern char gTrackImage[80];

int RunMediaPlayer(int track, short *endRollOrWord, short *imageParts);
int PlayMediaTrack(void);
void LoadDecodedSlice(void);
MovieFrame *GetMovieFrame(MovieDecoder *dec);
int StartXaTrack(int channel);
void PollXaPlayback(void);
void DecodeNextMovieFrame();
void StartMovie(int sector);
void PlayIdleMovie(void);
void PlayIdleAudio(void);
int PlayEndRoll(void);
void ShowTrackImage(int frame);
int LoadTrackImages(void);
void UpdateTrackImage(void);
int StartStreamRead(u_char *location);
void ReadDecodedAudio(void);
void SwapSpuIrqAddr(void);
int StopMovie(void);
void PlayIdleMedia(void);
int RestartXaRead(int command);
int StartXaRead(int start, int end);
void MuteCdAudio(void);
void UnmuteCdAudio(void);
int GetCdPeakLevel(void);
int SeekXaFile(void);
int InitSprite(GsSPRITE *sprite, int x, int y, int w, int h, int tpage, int u, int v, int cx, int cy, u_long attribute, int mx, int my);
int IsTrackDone(void);

#endif
