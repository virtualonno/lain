#ifndef GRID_EFFECTS_H
#define GRID_EFFECTS_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* 212 bytes: a line from its head coordinate to its tail. The head moves by
   speed from start until it passes end; the tail trails it by length. */
typedef struct {
    GsCOORDINATE2 head;
    GsCOORDINATE2 tail;
    SVECTOR unused;             /* 8 bytes nothing reads */
    GsGLINE line;
    SVECTOR start;
    SVECTOR end;
    short speed;
    short length;
    u_char r;
    u_char g;
    u_char b;
} Streak;

enum RingMotion {
    RING_MOTION_IDLE,
    RING_MOTION_LEVEL_UP,
    RING_MOTION_LEVEL_DOWN,
    RING_MOTION_RISE_OVERSHOOT,
    RING_MOTION_DROP_OVERSHOOT,
    RING_MOTION_SETTLE_FROM_ABOVE,
    RING_MOTION_SETTLE_FROM_BELOW,
    RING_MOTION_TURN_LEFT,
    RING_MOTION_TURN_RIGHT,
    RING_MOTION_RETURN_FROM_LEFT,
    RING_MOTION_RETURN_FROM_RIGHT
};
extern int gRingMotion;
enum RingRippleMode {
    RING_RIPPLE_WAVE,
    RING_RIPPLE_RESET,
    RING_RIPPLE_SCATTER
};
/* RING_RIPPLE_RESET falls into the default arm, writing unrippled vertices and resetting phase and shift. */
extern int gRingRippleMode;
extern int gRingRipplePhase;
extern int gRingRippleShift;
extern short gRingTiltDegrees;
extern int gRingTilt;
extern int gRingDipRollAt;
extern int gRingRippleDecayAt;
extern SVECTOR gCubeCorners[8];
extern SVECTOR gCubePieceStarts[8];
extern SVECTOR gRingFlatPoints[64];
extern SVECTOR gRingDipPoints[64];
extern SVECTOR gRingBentDownPoints[64];
extern SVECTOR gRingBentUpPoints[64];
extern VECTOR gQuadOrbitPoint;
extern u_char gStreakColors[4][3];

void InitRing(void);
void InitPieceBurst(void);
void StepRing(void);
void StepPieceBurst(void);
void RotateRingTimColumnsRight(void);
void CopyPoints(SVECTOR *dst, const SVECTOR *src, int count);
void StartPointMorph(const SVECTOR *points, unsigned int mode, int steps);
void InitTexturedQuad(void);
void DrawOrbitingTexturedQuad(void);
void DrawExpandingTexturedQuad(void);
void StartExpandingTexturedQuad(void);
void InitHorizonAndStreaks(void);
void StepHorizonAndStreaks(void);
void InitVerticalStreak(Streak *rec);
void InitPathStreak(Streak *rec, SVECTOR *path);
void StepStreakNegXNegZ(Streak *rec);
void StepStreakPosXNegZ(Streak *rec);
void StepVerticalStreak(Streak *rec);

extern SVECTOR gLargeSpherePoints[80];
extern SVECTOR gSmallSpherePoints[84];

#endif
