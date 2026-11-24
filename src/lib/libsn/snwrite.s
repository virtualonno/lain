; LIBSN SNWRITE: the BREAK handler returns status in v0 and the write result in v1.
	opt	c+

	xdef	_SN_write
	section	.text

_SN_write:
	break	$106
	beq	v0,zero,.Ldone
	addu	v0,v1,zero
	addiu	v0,zero,-1
.Ldone:
	jr	ra
	nop
