	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetRGBcd

	section	.text

SetRGBcd:
	lwc2	C2_RGB,0(a0)
	jr	ra
	nop
	nop
