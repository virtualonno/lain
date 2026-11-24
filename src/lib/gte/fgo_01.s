	opt	c+,at-,m-
#include <hilo.h>

	xdef	RotMatrix
	xref	rcossin_tbl

; Each Q12 table word packs sine below cosine.
lookup_pair	macro index
	sll	t8,index,2
	lui	t9,HI(rcossin_tbl)
	addu	t9,t9,t8
	lw	t9,LO(rcossin_tbl)(t9)
	nop
	endm

	section	.text

RotMatrix:
	lh	t7,0(a0)
	addu	v0,a1,zero
	bgez	t7,.positive_x
	andi	t9,t7,$fff
	subu	t7,zero,t7
	bgez	t7,.lookup_negative_x
	andi	t7,t7,$fff
.lookup_negative_x:
	lookup_pair t7
	sll	t8,t9,16
	sra	t8,t8,16
	subu	t3,zero,t8
	j	.read_y
	sra	t0,t9,16
.positive_x:
	lookup_pair t9
	sll	t8,t9,16
	sra	t3,t8,16
	sra	t0,t9,16
.read_y:
	lh	t7,2(a0)
	nop
	bgez	t7,.positive_y
	andi	t9,t7,$fff
	subu	t7,zero,t7
	bgez	t7,.lookup_negative_y
	andi	t7,t7,$fff
.lookup_negative_y:
	lookup_pair t7
	sll	t4,t9,16
	sra	t4,t4,16
	subu	t6,zero,t4
	j	.read_z
	sra	t1,t9,16
.positive_y:
	lookup_pair t9
	sll	t6,t9,16
	sra	t6,t6,16
	subu	t4,zero,t6
	sra	t1,t9,16
.read_z:
	multu	t1,t3
	lh	t7,4(a0)
	sh	t6,4(a1)
	mflo	t8
	subu	t9,zero,t8
	sra	t6,t9,12
	multu	t1,t0
	sh	t6,10(a1)
	bgez	t7,.positive_z
	andi	t9,t7,$fff
	mflo	t8
	sra	t6,t8,12
	sh	t6,16(a1)
	subu	t7,zero,t7
	bgez	t7,.lookup_negative_z
	andi	t7,t7,$fff
.lookup_negative_z:
	lookup_pair t7
	sll	t8,t9,16
	sra	t8,t8,16
	subu	t5,zero,t8
	j	.combine
	sra	t2,t9,16
.positive_z:
	mflo	t7
	sra	t6,t7,12
	sh	t6,16(a1)
	lookup_pair t9
	sll	t8,t9,16
	sra	t5,t8,16
	sra	t2,t9,16
.combine:
	multu	t2,t1
	nop
	nop
	mflo	t7
	sra	t6,t7,12
	sh	t6,0(a1)
	multu	t5,t1
	nop
	nop
	mflo	t7
	subu	t6,zero,t7
	sra	t7,t6,12
	multu	t2,t4
	sh	t7,2(a1)
	nop
	mflo	t7
	sra	t8,t7,12
	nop
	multu	t8,t3
	nop
	nop
	mflo	t7
	sra	t6,t7,12
	nop
	multu	t5,t0
	nop
	nop
	mflo	t7
	sra	t9,t7,12
	subu	t7,t9,t6
	multu	t8,t0
	sh	t7,6(a1)
	nop
	mflo	t6
	sra	t7,t6,12
	nop
	multu	t5,t3
	nop
	nop
	mflo	t6
	sra	t9,t6,12
	addu	t6,t9,t7
	multu	t5,t4
	sh	t6,12(a1)
	nop
	mflo	t7
	sra	t8,t7,12
	nop
	multu	t8,t3
	nop
	nop
	mflo	t7
	sra	t6,t7,12
	nop
	multu	t2,t0
	nop
	nop
	mflo	t7
	sra	t9,t7,12
	addu	t7,t9,t6
	multu	t8,t0
	sh	t7,8(a1)
	nop
	mflo	t6
	sra	t7,t6,12
	nop
	multu	t2,t3
	nop
	nop
	mflo	t6
	sra	t9,t6,12
	subu	t6,t9,t7
	sh	t6,14(a1)
	jr	ra
	nop
	nop
