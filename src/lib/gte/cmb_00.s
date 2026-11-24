	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	RotTransPers4

	section	.text

RotTransPers4:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	nop
	wRTPT
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	swc2	C2_SXY0,0(t0)
	swc2	C2_SXY1,0(t1)
	swc2	C2_SXY2,0(t2)
	cfc2	v1,C2_FLAG
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	nop
	wRTPS
	lw	t0,28(sp)
	lw	t1,32(sp)
	lw	t2,36(sp)
	swc2	C2_SXY2,0(t0)
	swc2	C2_IR0,0(t1)
	cfc2	t0,C2_FLAG
	mfc2	v0,C2_SZ3
	or	t0,t0,v1
	sw	t0,0(t2)
	jr	ra
	sra	v0,v0,2
	nop
	nop
