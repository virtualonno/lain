; LIBSN host-file close trap.
	opt	c+

	xdef	PCclose

	section	.text

PCclose:
	addu	a1,a0,zero
	break	$104
	jr	ra
	nop
