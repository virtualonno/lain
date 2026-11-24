	opt	c+,at-,m-
#include <gtereg.h>
	include	"INLINE_A.H"
	include	"gte_word.inc"

	xdef	LocalLight
	xdef	DpqColor
	xdef	NormalColor
	xdef	NormalColor3
	xdef	NormalColorDpq
	xdef	NormalColorDpq3
	xdef	NormalColorCol
	xdef	NormalColorCol3
	xdef	ColorDpq
	xdef	ColorCol
	xdef	AverageSZ3
	xdef	AverageSZ4

	section	.text

LocalLight:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	nop
	wMVMVA	1,1,0,3,1
	swc2	C2_IR1,0(a1)
	swc2	C2_IR2,4(a1)
	swc2	C2_IR3,8(a1)
	jr	ra
	nop

DpqColor:
	lwc2	C2_RGB,0(a0)
	mtc2	a1,C2_IR0
	nop
	wDPCS
	swc2	C2_RGB2,0(a2)
	jr	ra
	nop

NormalColor:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	nop
	wNCS
	swc2	C2_RGB2,0(a1)
	jr	ra
	nop

NormalColor3:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	nop
	wNCT
	lw	t0,16(sp)
	lw	t1,20(sp)
	swc2	C2_RGB0,0(a3)
	swc2	C2_RGB1,0(t0)
	swc2	C2_RGB2,0(t1)
	jr	ra
	nop

NormalColorDpq:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_RGB,0(a1)
	mtc2	a2,C2_IR0
	nop
	wNCDS
	swc2	C2_RGB2,0(a3)
	jr	ra
	nop

NormalColorDpq3:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	lwc2	C2_RGB,0(a3)
	lwc2	C2_IR0,16(sp)
	nop
	wNCDT
	lw	t0,20(sp)
	lw	t1,24(sp)
	lw	t2,28(sp)
	swc2	C2_RGB0,0(t0)
	swc2	C2_RGB1,0(t1)
	swc2	C2_RGB2,0(t2)
	jr	ra
	nop

NormalColorCol:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_RGB,0(a1)
	nop
	wNCCS
	swc2	C2_RGB2,0(a2)
	jr	ra
	nop

NormalColorCol3:
	lwc2	C2_VXY0,0(a0)
	lwc2	C2_VZ0,4(a0)
	lwc2	C2_VXY1,0(a1)
	lwc2	C2_VZ1,4(a1)
	lwc2	C2_VXY2,0(a2)
	lwc2	C2_VZ2,4(a2)
	lwc2	C2_RGB,0(a3)
	nop
	wNCCT
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	swc2	C2_RGB0,0(t0)
	swc2	C2_RGB1,0(t1)
	swc2	C2_RGB2,0(t2)
	jr	ra
	nop

ColorDpq:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	lwc2	C2_RGB,0(a1)
	mtc2	a2,C2_IR0
	nop
	wCDP
	swc2	C2_RGB2,0(a3)
	jr	ra
	nop

ColorCol:
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	lwc2	C2_RGB,0(a1)
	nop
	wCC
	swc2	C2_RGB2,0(a2)
	jr	ra
	nop

AverageSZ3:
	AVSZ3
	mfc2	v0,C2_OTZ
	jr	ra
	nop

AverageSZ4:
	AVSZ4
	mfc2	v0,C2_OTZ
	jr	ra
	nop
	nop
	nop
	nop
