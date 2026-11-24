	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	RotAverageNclipColorCol3

	section	.text

RotAverageNclipColorCol3:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	nop
	wRTPT
	lw	t0,56(sp)
	cfc2	t1,C2_FLAG
	nop
	sw	t1,0(t0)
	NCLIP
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	mfc2	v0,C2_MAC0
	nop
	bgtz	v0,.front
	nop
	beq	zero,zero,.return
.front:
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	lwc2	C2_VXY1,0(t0)
	lwc2	C2_VZ1,4(t0)
	lwc2	C2_VXY2,0(t1)
	lwc2	C2_VZ2,4(t1)
	lwc2	C2_RGB,0(t2)
	nop
	wNCCT
	lw	t0,28(sp)
	lw	t1,32(sp)
	lw	t2,36(sp)
	lw	t3,40(sp)
	lw	t4,44(sp)
	lw	t5,48(sp)
	swc2	C2_SXY0,0(t0)
	swc2	C2_SXY1,0(t1)
	swc2	C2_SXY2,0(t2)
	swc2	C2_RGB0,0(t3)
	swc2	C2_RGB1,0(t4)
	swc2	C2_RGB2,0(t5)
	nop
	wAVSZ3
	lw	t1,52(sp)
	mfc2	t0,C2_OTZ
	nop
	sw	t0,0(t1)
.return:
	jr	ra
	nop
	nop
	nop
