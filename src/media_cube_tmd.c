/* TMD models of the media player cube's Play and Exit pieces, plain and selected, of the tetrahedron
 * at each level's text band, and of a band nothing reads. */
#include <sys/types.h>
#include "media_cube_tmd.h"

u_long gExitPieceTmd[86] = {
#include "content/gExitPieceTmd.inc"
};

u_long gExitPieceSelectedTmd[86] = {
#include "content/gExitPieceSelectedTmd.inc"
};

u_long gPlayPieceTmd[68] = {
#include "content/gPlayPieceTmd.inc"
};

u_long gPlayPieceSelectedTmd[68] = {
#include "content/gPlayPieceSelectedTmd.inc"
};

/* 3880 bytes; nothing reads it */
u_long gUnusedBandTmd[970] = {
#include "content/gUnusedBandTmd.inc"
};

u_long gTextBandTetrahedronTmd[42] = {
#include "content/gTextBandTetrahedronTmd.inc"
};
