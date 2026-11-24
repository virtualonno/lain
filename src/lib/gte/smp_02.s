	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	RotTransPers

	section	.text

RotTransPers:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	nop
	wRTPS
	swc2	C2_SXY2,0(a1)
	swc2	C2_IR0,0(a2)
	cfc2	v1,C2_FLAG
	mfc2	v0,C2_SZ3
	sw	v1,0(a3)
	jr	ra
	sra	v0,v0,2
	nop
