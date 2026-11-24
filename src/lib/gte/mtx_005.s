	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	ApplyRotMatrix

	section	.text

ApplyRotMatrix:
	lw	t0,0(a0)
	lw	t1,4(a0)
	mtc2	t0,C2_VXY0
	mtc2	t1,C2_VZ0
	nop
	wMVMVA 1,0,0,3,0
	swc2	C2_IR1,0(a1)
	swc2	C2_IR2,4(a1)
	swc2	C2_IR3,8(a1)
	addu	v0,a1,zero
	jr	ra
	nop
