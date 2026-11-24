	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	RotTrans

	section	.text

RotTrans:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	nop
	wMVMVA	1,0,0,0,0
	swc2	C2_MAC1,0(a1)
	swc2	C2_MAC2,4(a1)
	swc2	C2_MAC3,8(a1)
	cfc2	v0,C2_FLAG
	jr	ra
	sw	v0,0(a2)
	nop
	nop
