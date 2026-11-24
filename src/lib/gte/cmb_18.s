	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	RotAverageNclipColorDpq3_1

; Use each vertex's own perspective cue for its lighting calculation.
light_vertex	macro cue,xy_dest,rgb_dest,screen_reg
	mtc2	cue,C2_IR0
	nop
	wNCDS
	lw	t0,xy_dest(sp)
	lw	t1,rgb_dest(sp)
	swc2	screen_reg,0(t0)
	swc2	C2_RGB2,0(t1)
	endm

	section	.text

RotAverageNclipColorDpq3_1:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	nop
	wRTPS
	mfc2	t7,C2_IR0
	lwc2	C2_VXY0,0(a1)
	lwc2	C2_VZ0,4(a1)
	nop
	wRTPS
	mfc2	t8,C2_IR0
	lwc2	C2_VXY0,0(a2)
	lwc2	C2_VZ0,4(a2)
	nop
	wRTPS
	lw	v0,56(sp)
	mfc2	t9,C2_IR0
	cfc2	v1,C2_FLAG
	nop
	sw	v1,0(v0)
	NCLIP
	lw	t3,16(sp)
	lw	t4,20(sp)
	lw	t5,24(sp)
	mfc2	v0,C2_MAC0
	nop
	bgtz	v0,.front
	nop
	beq	zero,zero,.return
	nop
.front:
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	lwc2	C2_RGB,0(t5)
	light_vertex t7,28,40,C2_SXY0
	lwc2	C2_VXY0,0(t3)
	lwc2	C2_VZ0,4(t3)
	light_vertex t8,32,44,C2_SXY1
	lwc2	C2_VXY0,0(t4)
	lwc2	C2_VZ0,4(t4)
	light_vertex t9,36,48,C2_SXY2
	wAVSZ3
	lw	t1,52(sp)
	mfc2	t0,C2_OTZ
	nop
	sw	t0,0(t1)
.return:
	jr	ra
	nop
