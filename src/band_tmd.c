/* TMD models of the grid's text band and label band, and of a slab nothing reads. */
#include <sys/types.h>
#include "band_tmd.h"

/* 344 bytes; nothing reads it */
u_long gUnusedSlabTmd[86] = {
#include "content/gUnusedSlabTmd.inc"
};

u_long gTextBandTmd[970] = {
#include "content/gTextBandTmd.inc"
};

u_long gLabelBandTmd[754] = {
#include "content/gLabelBandTmd.inc"
};
