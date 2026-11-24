	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	MulMatrix2

	section	.text

MulMatrix2:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)
	ctc2	t0,C2_R11R12
	ctc2	t1,C2_R13R21
	ctc2	t2,C2_R22R23
	ctc2	t3,C2_R31R32
	ctc2	t4,C2_R33
	lhu	t0,0(a1)
	lw	t1,4(a1)
	lw	t2,12(a1)
	lui	at,$ffff
	and	t1,t1,at
	or	t0,t0,t1
	mtc2	t0,C2_VXY0
	mtc2	t2,C2_VZ0
	nop
	wMVMVA	1,0,0,3,0
	lhu	t0,2(a1)
	lw	t1,8(a1)
	lh	t2,14(a1)
	sll	t1,t1,16
	or	t0,t0,t1
	mfc2	t3,C2_IR1
	mfc2	t4,C2_IR2
	mfc2	t5,C2_IR3
	mtc2	t0,C2_VXY0
	mtc2	t2,C2_VZ0
	nop
	wMVMVA	1,0,0,3,0
	lhu	t0,4(a1)
	lw	t1,8(a1)
	lw	t2,16(a1)
	lui	at,$ffff
	and	t1,t1,at
	or	t0,t0,t1
	mfc2	t6,C2_IR1
	mfc2	t7,C2_IR2
	mfc2	t8,C2_IR3
	mtc2	t0,C2_VXY0
	mtc2	t2,C2_VZ0
	nop
	wMVMVA	1,0,0,3,0
	andi	t3,t3,$ffff
	sll	t6,t6,16
	or	t6,t6,t3
	sw	t6,0(a1)
	andi	t5,t5,$ffff
	sll	t8,t8,16
	or	t8,t8,t5
	sw	t8,12(a1)
	mfc2	t0,C2_IR1
	mfc2	t1,C2_IR2
	andi	t0,t0,$ffff
	sll	t4,t4,16
	or	t0,t0,t4
	sw	t0,4(a1)
	andi	t7,t7,$ffff
	sll	t1,t1,16
	or	t1,t1,t7
	sw	t1,8(a1)
	swc2	C2_IR3,16(a1)
	addu	v0,a1,zero
	jr	ra
	nop
	nop
