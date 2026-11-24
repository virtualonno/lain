	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetLightMatrix

	section	.text

SetLightMatrix:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)
	ctc2	t0,C2_L11L12
	ctc2	t1,C2_L13L21
	ctc2	t2,C2_L22L23
	ctc2	t3,C2_L31L32
	ctc2	t4,C2_L33
	jr	ra
	nop
