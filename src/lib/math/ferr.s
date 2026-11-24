; LIBMATH FERR
	opt	c+,at-,m+

	xref	DeliverEvent
	xdef	math_errno
	xdef	math_err_point
	xdef	_err_math

	section	.text

_err_math:
	addiu	sp,sp,-24
	addiu	v0,zero,33
	sw	ra,16(sp)
	opt	at+
	sw	a0,math_errno
	opt	at-
	lui	at,((math_err_point)+$8000)>>16
	opt	m-
	beq	a0,v0,.else
	sw	a1,((math_err_point)-((((math_err_point)+$8000)>>16)<<16))(at)
	opt	m+
	addiu	v0,zero,34
	beq	a0,v0,.else_2
	lui	a0,$f400
	j	.return
	nop
.else:
	li	a0,$f4000002
	j	.skip
	addiu	a1,zero,769
.else_2:
	ori	a0,a0,$2
	addiu	a1,zero,770
.skip:
	jal	DeliverEvent
	nop
.return:
	lw	ra,16(sp)
	addu	v0,zero,zero
	jr	ra
	addiu	sp,sp,24
	nop
	nop
	nop

	section	.data
	dw	$250b7350
	dw	$0043539b
math_errno:
	dw	$00000000
math_err_point:
	dw	$00000000
