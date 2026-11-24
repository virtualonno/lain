/* TMD model of the grid's morphing ring, and a quad TMD nothing reads. */
#include <sys/types.h>
#include "ring_tmd.h"

/* 112 bytes; nothing reads it */
u_long gUnusedQuadTmd[28] = {
#include "content/gUnusedQuadTmd.inc"
};

u_long gRingTmd[970] = {
#include "content/gRingTmd.inc"
};
