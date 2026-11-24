	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetTransMatrix

	section	.text

SetTransMatrix:
	lw	t0,20(a0)
	lw	t1,24(a0)
	lw	t2,28(a0)
	ctc2	t0,C2_TRX
	ctc2	t1,C2_TRY
	ctc2	t2,C2_TRZ
	jr	ra
	nop
