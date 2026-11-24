#ifndef LIBGS_INT_H
#define LIBGS_INT_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern long GsCLIP3far;
extern long GsCLIP3near;
extern GsCOORDINATE2 *DSTACK[32];
extern RECT CLIP2;
extern short GsORGOFSX;
extern short GsORGOFSY;
extern MATRIX _LC;
extern short PSDBASEX[2];
extern short PSDBASEY[2];
extern u_long PSDCNT;
extern short PSDGPU;
extern short PSDIDX;
extern short PSDOFSX[2];
extern short PSDOFSY[2];

void GsSetFarClip(long farClip);
void GsSetNearClip(long nearClip);
void gte_rotate_z_matrix(MATRIX *m, int rz);
PACKET *_make_packet(PACKET *packet, GsOT *ot, u_short pri, int words);
void gte_init(void);
void Gssub_make_matrix(MATRIX *m, short s, short c, char axis);

#endif
