	opt	c+,at-,m-

	xdef	TransMatrix

	section	.text

TransMatrix:
	lw	t0,0(a1)
	lw	t1,4(a1)
	lw	t2,8(a1)
	sw	t0,20(a0)
	sw	t1,24(a0)
	sw	t2,28(a0)
	addu	v0,a0,zero
	jr	ra
	nop
	nop
	nop
	nop
