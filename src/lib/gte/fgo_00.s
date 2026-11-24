	opt	c+,at-,m-

	xdef	TransposeMatrix

	section	.text

TransposeMatrix:
	addu	v0,a1,zero
	lw	t1,0(a0)
	lw	t2,4(a0)
	sw	t1,4(a1)
	sw	t2,0(a1)
	sh	t1,0(a1)
	lw	t3,8(a0)
	lw	t1,12(a0)
	sw	t3,12(a1)
	sw	t1,8(a1)
	sh	t2,12(a1)
	sh	t3,8(a1)
	lh	t2,16(a0)
	sh	t1,4(a1)
	jr	ra
	sh	t2,16(a1)
