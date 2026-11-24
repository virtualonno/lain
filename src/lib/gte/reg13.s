	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetGeomScreen

	section	.text

SetGeomScreen:
	ctc2	a0,C2_H
	jr	ra
	nop
	nop
