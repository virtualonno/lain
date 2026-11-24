	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	LightColor
	xdef	DpqColorLight
	xdef	DpqColor3
	xdef	Intpl
	xdef	Square12
	xdef	Square0
	xdef	AverageZ3
	xdef	AverageZ4
	xdef	OuterProduct12
	xdef	OuterProduct0
	xdef	Lzc

	section	.text

LightColor:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	nop
	wMVMVA	1,2,3,1,1
	swc2	C2_IR1,0(a1)
	swc2	C2_IR2,4(a1)
	swc2	C2_IR3,8(a1)
	jr	ra
	nop

DpqColorLight:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	lwc2	C2_RGB,0(a1)
	mtc2	a2,C2_IR0
	nop
	wDCPL
	swc2	C2_RGB2,0(a3)
	jr	ra
	nop

DpqColor3:
	lwc2	C2_RGB0,0(a0)
	lwc2	C2_RGB1,0(a1)
	lwc2	C2_RGB2,0(a2)
	lwc2	C2_RGB,0(a2)
	mtc2	a3,C2_IR0
	nop
	wDPCT
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	swc2	C2_RGB0,0(t0)
	swc2	C2_RGB1,0(t1)
	swc2	C2_RGB2,0(t2)
	jr	ra
	nop

Intpl:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	mtc2	a1,C2_IR0
	nop
	wINTPL
	swc2	C2_RGB2,0(a2)
	jr	ra
	nop

Square12:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	nop
	wSQR	1
	swc2	C2_MAC1,0(a1)
	swc2	C2_MAC2,4(a1)
	swc2	C2_MAC3,8(a1)
	jr	ra
	addu	v0,a1,zero

Square0:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	nop
	wSQR	0
	swc2	C2_MAC1,0(a1)
	swc2	C2_MAC2,4(a1)
	swc2	C2_MAC3,8(a1)
	jr	ra
	addu	v0,a1,zero

AverageZ3:
	mtc2	a0,C2_SZ1
	mtc2	a1,C2_SZ2
	mtc2	a2,C2_SZ3
	nop
	wAVSZ3
	mfc2	v0,C2_OTZ
	jr	ra
	nop

AverageZ4:
	mtc2	a0,C2_SZ0
	mtc2	a1,C2_SZ1
	mtc2	a2,C2_SZ2
	mtc2	a3,C2_SZ3
	nop
	wAVSZ4
	mfc2	v0,C2_OTZ
	jr	ra
	nop

OuterProduct12:
	cfc2	t5,C2_R11R12
	cfc2	t6,C2_R22R23
	cfc2	t7,C2_R33
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	ctc2	t0,C2_R11R12
	ctc2	t1,C2_R22R23
	ctc2	t2,C2_R33
	lwc2	C2_IR3,8(a1)
	lwc2	C2_IR1,0(a1)
	lwc2	C2_IR2,4(a1)
	nop
	wOP	1
	swc2	C2_MAC1,0(a2)
	swc2	C2_MAC2,4(a2)
	swc2	C2_MAC3,8(a2)
	ctc2	t5,C2_R11R12
	ctc2	t6,C2_R22R23
	ctc2	t7,C2_R33
	jr	ra
	nop

OuterProduct0:
	cfc2	t5,C2_R11R12
	cfc2	t6,C2_R22R23
	cfc2	t7,C2_R33
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	ctc2	t0,C2_R11R12
	ctc2	t1,C2_R22R23
	ctc2	t2,C2_R33
	lwc2	C2_IR3,8(a1)
	lwc2	C2_IR1,0(a1)
	lwc2	C2_IR2,4(a1)
	nop
	wOP	0
	swc2	C2_MAC1,0(a2)
	swc2	C2_MAC2,4(a2)
	swc2	C2_MAC3,8(a2)
	ctc2	t5,C2_R11R12
	ctc2	t6,C2_R22R23
	ctc2	t7,C2_R33
	jr	ra
	nop

Lzc:
	mtc2	a0,C2_LZCS
	nop
	nop
	mfc2	v0,C2_LZCR
	jr	ra
	nop
	nop
