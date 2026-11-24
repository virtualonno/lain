	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	LoadAverage12
	xdef	LoadAverage0
	xdef	LoadAverageShort12
	xdef	LoadAverageShort0
	xdef	LoadAverageByte
	xdef	LoadAverageCol

	section	.text

LoadAverage12:
	mtc2	a2,C2_IR0
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	nop
	wGPF	1
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	lwc2	C2_IR1,0(a1)
	lwc2	C2_IR2,4(a1)
	lwc2	C2_IR3,8(a1)
	nop
	wGPL	1
	lw	t0,16(sp)
	nop
	swc2	C2_IR1,0(t0)
	swc2	C2_IR2,4(t0)
	swc2	C2_IR3,8(t0)
	jr	ra
	nop

LoadAverage0:
	mtc2	a2,C2_IR0
	lwc2	C2_IR1,0(a0)
	lwc2	C2_IR2,4(a0)
	lwc2	C2_IR3,8(a0)
	nop
	wGPF	0
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	lwc2	C2_IR1,0(a1)
	lwc2	C2_IR2,4(a1)
	lwc2	C2_IR3,8(a1)
	nop
	wGPL	0
	lw	t0,16(sp)
	nop
	swc2	C2_IR1,0(t0)
	swc2	C2_IR2,4(t0)
	swc2	C2_IR3,8(t0)
	jr	ra
	nop

LoadAverageShort12:
	lw	t0,0(a0)
	lw	t2,4(a0)
	sra	t1,t0,16
	andi	t0,t0,$ffff
	andi	t2,t2,$ffff
	mtc2	a2,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wGPF	1
	lw	t0,0(a1)
	lw	t2,4(a1)
	sra	t1,t0,16
	andi	t0,t0,$ffff
	andi	t2,t2,$ffff
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wGPL	1
	mfc2	t0,C2_IR1
	mfc2	t1,C2_IR2
	andi	t0,t0,$ffff
	sll	t1,t1,16
	or	t0,t0,t1
	lw	t5,16(sp)
	mfc2	t2,C2_IR3
	sw	t0,0(t5)
	sw	t2,4(t5)
	jr	ra
	nop

LoadAverageShort0:
	lw	t0,0(a0)
	lw	t2,4(a0)
	sra	t1,t0,16
	andi	t0,t0,$ffff
	andi	t2,t2,$ffff
	mtc2	a2,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wGPF	0
	lw	t0,0(a1)
	lw	t2,4(a1)
	sra	t1,t0,16
	andi	t0,t0,$ffff
	andi	t2,t2,$ffff
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wGPL	0
	mfc2	t0,C2_IR1
	mfc2	t1,C2_IR2
	andi	t0,t0,$ffff
	sll	t1,t1,16
	or	t0,t0,t1
	lw	t5,16(sp)
	mfc2	t2,C2_IR3
	sw	t0,0(t5)
	sw	t2,4(t5)
	jr	ra
	nop

LoadAverageByte:
	lbu	t0,0(a0)
	lbu	t1,1(a0)
	mtc2	a2,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	nop
	wGPF	0
	lbu	t0,0(a1)
	lbu	t1,1(a1)
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	addiu	t3,zero,12
	wGPL	0
	lw	t5,16(sp)
	mfc2	t0,C2_MAC1
	mfc2	t1,C2_MAC2
	srav	t0,t0,t3
	srav	t1,t1,t3
	sb	t0,0(t5)
	sb	t1,1(t5)
	jr	ra
	nop

LoadAverageCol:
	lbu	t0,0(a0)
	lbu	t1,1(a0)
	lbu	t2,2(a0)
	mtc2	a2,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wGPF	0
	lbu	t0,0(a1)
	lbu	t1,1(a1)
	lbu	t2,2(a1)
	mfc2	v0,C2_LZCR
	mtc2	a3,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	addiu	t3,zero,12
	wGPL	0
	lw	t5,16(sp)
	mfc2	t0,C2_MAC1
	mfc2	t1,C2_MAC2
	mfc2	t2,C2_MAC3
	srav	t0,t0,t3
	srav	t1,t1,t3
	srav	t2,t2,t3
	sb	t0,0(t5)
	sb	t1,1(t5)
	sb	t2,2(t5)
	jr	ra
	nop
	nop
