	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetFarColor

	section	.text

SetFarColor:
	sll	a0,a0,4
	sll	a1,a1,4
	sll	a2,a2,4
	ctc2	a0,C2_RFC
	ctc2	a1,C2_GFC
	ctc2	a2,C2_BFC
	jr	ra
	nop
