	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	ApplyMatrix

	section	.text

ApplyMatrix:
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
	lwc2	C2_VXY0,0(a1)
	lwc2	C2_VZ0,4(a1)
	nop
	wMVMVA	1,0,0,3,0
	swc2	C2_MAC1,0(a2)
	swc2	C2_MAC2,4(a2)
	swc2	C2_MAC3,8(a2)
	addu	v0,a2,zero
	jr	ra
	nop
