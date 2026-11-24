/* TMD models: the grid's node cells, opened and not, the empty cell, the cursor highlight, the four
 * pieces a cell splits into and the 32 burst pieces; the end-roll spheres; the gate scene's slabs
 * and planes. */
#include <sys/types.h>

#include "tmd_data_5.h"

u_long gLainCellTmd[86] = {
#include "content/gLainCellTmd.inc"
};

u_long gLainDarkCellTmd[86] = {
#include "content/gLainDarkCellTmd.inc"
};

u_long gToukoCellTmd[86] = {
#include "content/gToukoCellTmd.inc"
};

u_long gLabCellTmd[86] = {
#include "content/gLabCellTmd.inc"
};

u_long gSaiseikunCellTmd[86] = {
#include "content/gSaiseikunCellTmd.inc"
};

u_long gTransferCellTmd[86] = {
#include "content/gTransferCellTmd.inc"
};

u_long gMovieCellTmd[86] = {
#include "content/gMovieCellTmd.inc"
};

u_long gPsychiatryCellTmd[86] = {
#include "content/gPsychiatryCellTmd.inc"
};

u_long gEmptyCellTmd[86] = {
#include "content/gEmptyCellTmd.inc"
};

u_long gCursorHighlightTmd[86] = {
#include "content/gCursorHighlightTmd.inc"
};

u_long gCellPiece0Tmd[50] = {
#include "content/gCellPiece0Tmd.inc"
};

u_long gCellPiece1Tmd[50] = {
#include "content/gCellPiece1Tmd.inc"
};

u_long gCellPiece2Tmd[50] = {
#include "content/gCellPiece2Tmd.inc"
};

u_long gCellPiece3Tmd[50] = {
#include "content/gCellPiece3Tmd.inc"
};

u_long gOpenedLainCellTmd[86] = {
#include "content/gOpenedLainCellTmd.inc"
};

u_long gOpenedLainDarkCellTmd[86] = {
#include "content/gOpenedLainDarkCellTmd.inc"
};

u_long gOpenedToukoCellTmd[86] = {
#include "content/gOpenedToukoCellTmd.inc"
};

u_long gOpenedLabCellTmd[86] = {
#include "content/gOpenedLabCellTmd.inc"
};

u_long gOpenedSaiseikunCellTmd[86] = {
#include "content/gOpenedSaiseikunCellTmd.inc"
};

u_long gOpenedTransferCellTmd[86] = {
#include "content/gOpenedTransferCellTmd.inc"
};

u_long gOpenedMovieCellTmd[86] = {
#include "content/gOpenedMovieCellTmd.inc"
};

u_long gOpenedPsychiatryCellTmd[86] = {
#include "content/gOpenedPsychiatryCellTmd.inc"
};

u_long gBurstPiece00Tmd[52] = {
#include "content/gBurstPiece00Tmd.inc"
};

u_long gBurstPiece01Tmd[52] = {
#include "content/gBurstPiece01Tmd.inc"
};

u_long gBurstPiece02Tmd[52] = {
#include "content/gBurstPiece02Tmd.inc"
};

u_long gBurstPiece03Tmd[52] = {
#include "content/gBurstPiece03Tmd.inc"
};

u_long gBurstPiece04Tmd[52] = {
#include "content/gBurstPiece04Tmd.inc"
};

u_long gBurstPiece05Tmd[52] = {
#include "content/gBurstPiece05Tmd.inc"
};

u_long gBurstPiece06Tmd[52] = {
#include "content/gBurstPiece06Tmd.inc"
};

u_long gBurstPiece07Tmd[52] = {
#include "content/gBurstPiece07Tmd.inc"
};

u_long gBurstPiece08Tmd[52] = {
#include "content/gBurstPiece08Tmd.inc"
};

u_long gBurstPiece09Tmd[52] = {
#include "content/gBurstPiece09Tmd.inc"
};

u_long gBurstPiece10Tmd[52] = {
#include "content/gBurstPiece10Tmd.inc"
};

u_long gBurstPiece11Tmd[52] = {
#include "content/gBurstPiece11Tmd.inc"
};

u_long gBurstPiece12Tmd[52] = {
#include "content/gBurstPiece12Tmd.inc"
};

u_long gBurstPiece13Tmd[52] = {
#include "content/gBurstPiece13Tmd.inc"
};

u_long gBurstPiece14Tmd[52] = {
#include "content/gBurstPiece14Tmd.inc"
};

u_long gBurstPiece15Tmd[52] = {
#include "content/gBurstPiece15Tmd.inc"
};

u_long gBurstPiece16Tmd[52] = {
#include "content/gBurstPiece16Tmd.inc"
};

u_long gBurstPiece17Tmd[52] = {
#include "content/gBurstPiece17Tmd.inc"
};

u_long gBurstPiece18Tmd[52] = {
#include "content/gBurstPiece18Tmd.inc"
};

u_long gBurstPiece19Tmd[52] = {
#include "content/gBurstPiece19Tmd.inc"
};

u_long gBurstPiece20Tmd[52] = {
#include "content/gBurstPiece20Tmd.inc"
};

u_long gBurstPiece21Tmd[52] = {
#include "content/gBurstPiece21Tmd.inc"
};

u_long gBurstPiece22Tmd[52] = {
#include "content/gBurstPiece22Tmd.inc"
};

u_long gBurstPiece23Tmd[52] = {
#include "content/gBurstPiece23Tmd.inc"
};

u_long gBurstPiece24Tmd[52] = {
#include "content/gBurstPiece24Tmd.inc"
};

u_long gBurstPiece25Tmd[52] = {
#include "content/gBurstPiece25Tmd.inc"
};

u_long gBurstPiece26Tmd[52] = {
#include "content/gBurstPiece26Tmd.inc"
};

u_long gBurstPiece27Tmd[52] = {
#include "content/gBurstPiece27Tmd.inc"
};

u_long gBurstPiece28Tmd[52] = {
#include "content/gBurstPiece28Tmd.inc"
};

u_long gBurstPiece29Tmd[52] = {
#include "content/gBurstPiece29Tmd.inc"
};

u_long gBurstPiece30Tmd[52] = {
#include "content/gBurstPiece30Tmd.inc"
};

u_long gBurstPiece31Tmd[52] = {
#include "content/gBurstPiece31Tmd.inc"
};

u_long gLargeSphereTmd[1642] = {
#include "content/gLargeSphereTmd.inc"
};

u_long gSmallSphereTmd[1810] = {
#include "content/gSmallSphereTmd.inc"
};

u_long gGateSlab0Tmd[182] = {
#include "content/gGateSlab0Tmd.inc"
};

u_long gGateSlab1Tmd[182] = {
#include "content/gGateSlab1Tmd.inc"
};

u_long gGateSlab2Tmd[182] = {
#include "content/gGateSlab2Tmd.inc"
};

u_long gGateSlab3Tmd[182] = {
#include "content/gGateSlab3Tmd.inc"
};

u_long gGateFlyingSlabTmd[182] = {
#include "content/gGateFlyingSlabTmd.inc"
};

u_long gGateLeftPlaneTmd[812] = {
#include "content/gGateLeftPlaneTmd.inc"
};

u_long gGateRightPlaneTmd[812] = {
#include "content/gGateRightPlaneTmd.inc"
};
