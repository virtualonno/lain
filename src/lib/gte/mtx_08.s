	opt	c+,at-,m-

	xdef	ScaleMatrix

; A word of the packed matrix holds two signed Q12 coefficients. Each
; coefficient is scaled by the component for its column.
scale_pair	macro offset,low_scale,high_scale
	lw	t0,offset(a0)
	nop
	andi	t1,t0,$ffff
	sll	t1,t1,16
	sra	t1,t1,16
	multu	t1,low_scale
	mflo	t1
	sra	t1,t1,12
	andi	t1,t1,$ffff
	sra	t2,t0,16
	multu	t2,high_scale
	mflo	t2
	sra	t2,t2,12
	sll	t2,t2,16
	or	t1,t1,t2
	sw	t1,offset(a0)
	endm

	section	.text

ScaleMatrix:
	lw	t3,0(a1)
	lw	t4,4(a1)
	lw	t5,8(a1)
	scale_pair 0,t3,t4
	scale_pair 4,t5,t3
	scale_pair 8,t4,t5
	scale_pair 12,t3,t4
	lw	t0,16(a0)
	nop
	andi	t1,t0,$ffff
	sll	t1,t1,16
	sra	t1,t1,16
	multu	t1,t5
	mflo	t1
	sra	t1,t1,12
	sw	t1,16(a0)
	jr	ra
	addu	v0,a0,zero
	nop
	nop
