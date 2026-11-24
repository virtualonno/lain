	opt	c+,at-,m-
#include <gtereg.h>
	include	"INLINE_A.H"
	include	"gte_word.inc"

	xdef	RotAverageNclip4

	section	.text

RotAverageNclip4:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	nop
	wRTPT
	lw	t0,40(sp)
	cfc2	v1,C2_FLAG
	nop
	sw	v1,0(t0)
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
	swc2	C2_SXY0,0(t0)
	swc2	C2_SXY1,0(t1)
	swc2	C2_SXY2,0(t2)
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	nop
	wRTPS
	lw	t0,28(sp)
	lw	t1,32(sp)
	lw	t2,40(sp)
	swc2	C2_SXY2,0(t0)
	cfc2	t3,C2_FLAG
	swc2	C2_IR0,0(t1)
	or	t3,t3,v1
	sw	t3,0(t2)
	AVSZ4
	lw	t1,36(sp)
	mfc2	t0,C2_OTZ
	nop
	sw	t0,0(t1)
.Lreturn:
	jr	ra
	nop
