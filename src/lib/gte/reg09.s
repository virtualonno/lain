	opt	c+,at-,m-
#include <gtereg.h>

	xdef	ReadGeomScreen

	section	.text

ReadGeomScreen:
	cfc2	v0,C2_H
	jr	ra
	nop
	nop
