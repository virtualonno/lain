	opt	c+,at-,m-
#include <hilo.h>

	xref	rcossin_tbl
	xdef	RotMatrixX

	section	.text

RotMatrixX:
	addu	t7,a0,zero
	addu	v0,a1,zero
	bgez	t7,.Lpositive
	andi	t9,t7,$fff
	subu	t7,zero,t7
	bgez	t7,.Lnegative
	andi	t7,t7,$fff
.Lnegative:
	sll	t8,t7,2
	lui	t9,HI(rcossin_tbl)
	addu	t9,t9,t8
	lw	t9,LO(rcossin_tbl)(t9)
	nop
	sll	t6,t9,16
	sra	t6,t6,16
	subu	t1,zero,t6
	j	.Lrotate
	sra	t0,t9,16
.Lpositive:
	sll	t8,t9,2
	lui	t9,HI(rcossin_tbl)
	addu	t9,t9,t8
	lw	t9,LO(rcossin_tbl)(t9)
	nop
	sll	t8,t9,16
	sra	t1,t8,16
	sra	t0,t9,16
.Lrotate:
	lh	t2,6(a1)
	lh	t5,12(a1)
	multu	t0,t2
	lh	t3,8(a1)
	lh	t6,14(a1)
	mflo	t8
	lh	t4,10(a1)
	lh	t7,16(a1)
	multu	t1,t5
	nop
	nop
	mflo	t9
	subu	t9,t8,t9
	sra	t8,t9,12
	multu	t0,t3
	sh	t8,6(a1)
	nop
	mflo	t8
	nop
	nop
	multu	t1,t6
	nop
	nop
	mflo	t9
	subu	t9,t8,t9
	sra	t8,t9,12
	multu	t0,t4
	sh	t8,8(a1)
	nop
	mflo	t8
	nop
	nop
	multu	t1,t7
	nop
	nop
	mflo	t9
	subu	t9,t8,t9
	sra	t8,t9,12
	multu	t1,t2
	sh	t8,10(a1)
	nop
	mflo	t8
	nop
	nop
	multu	t0,t5
	nop
	nop
	mflo	t9
	addu	t9,t8,t9
	sra	t8,t9,12
	multu	t1,t3
	sh	t8,12(a1)
	nop
	mflo	t8
	nop
	nop
	multu	t0,t6
	nop
	nop
	mflo	t9
	addu	t9,t8,t9
	sra	t8,t9,12
	multu	t1,t4
	sh	t8,14(a1)
	nop
	mflo	t8
	nop
	nop
	multu	t0,t7
	nop
	nop
	mflo	t9
	addu	t9,t8,t9
	sra	t8,t9,12
	sh	t8,16(a1)
	jr	ra
	nop
	nop
	nop
