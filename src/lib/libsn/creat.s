; PsyQ host-file ABI: PCcreat(name, perms) ignores perms and passes zero
; in a2. The host returns status in v0 and the file descriptor in v1.
	opt	c+
	xdef	PCcreat

	section	.text

PCcreat:
	addu	a1,a0,zero
	addu	a2,zero,zero
	break	$102
	beq	v0,zero,.Lreturn
	addu	v0,v1,zero
	addiu	v0,zero,-1
.Lreturn:
	jr	ra
	nop
