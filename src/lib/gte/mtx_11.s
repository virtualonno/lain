	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetColorMatrix

	section	.text

SetColorMatrix:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)
	ctc2	t0,C2_LR1LR2
	ctc2	t1,C2_LR3LG1
	ctc2	t2,C2_LG2LG3
	ctc2	t3,C2_LB1LB2
	ctc2	t4,C2_LB3
	jr	ra
	nop
