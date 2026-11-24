	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetGeomOffset

	section	.text

SetGeomOffset:
	sll	a0,a0,16
	sll	a1,a1,16
	ctc2	a0,C2_OFX
	ctc2	a1,C2_OFY
	jr	ra
	nop
	nop
	nop
