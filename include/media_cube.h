#ifndef MEDIA_CUBE_H
#define MEDIA_CUBE_H

#include <sys/types.h>
#include <libgte.h>

extern SVECTOR gTextBandPosition;
extern SVECTOR gLabelBandPosition;
extern SVECTOR gRingRestPosition;
extern SVECTOR gRingRiseOvershoot;
extern SVECTOR gRingDropOvershoot;

void InitMediaPlayerCube(void);
void DrawMediaPlayerCube(short selection);

#endif
