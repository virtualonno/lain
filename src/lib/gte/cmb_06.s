	opt	c+,at-,m-
#include <gtereg.h>
	include	"INLINE_A.H"
	include	"gte_word.inc"

	xdef	RotAverageNclip3

	section	.text

RotAverageNclip3:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	nop
	wRTPT
	lw	t0,32(sp)
	cfc2	t1,C2_FLAG
	nop
	sw	t1,0(t0)
	NCLIP
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	mfc2	v0,C2_MAC0
	nop
	bgtz	v0,.Lfront
	nop
	beq	zero,zero,.Lreturn
	nop
.Lfront:
	swc2	C2_SXY0,0(a3)
	swc2	C2_SXY1,0(t0)
	swc2	C2_SXY2,0(t1)
	swc2	C2_IR0,0(t2)
	nop
	avsz3
	lw	t1,28(sp)
	mfc2	t0,C2_OTZ
	nop
	sw	t0,0(t1)
.Lreturn:
	jr	ra
	nop
	nop
	nop
