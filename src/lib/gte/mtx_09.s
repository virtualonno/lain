	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetRotMatrix

	section	.text

SetRotMatrix:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)
	ctc2	t0,C2_R11R12
	ctc2	t1,C2_R13R21
	ctc2	t2,C2_R22R23
	ctc2	t3,C2_R31R32
	ctc2	t4,C2_R33
	jr	ra
	nop
