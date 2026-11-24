; LIBAPI CHCLRPAD
	opt	c+,at+,m+

	xref	EnterCriticalSection
	xref	ExitCriticalSection
	xref	FlushCache
	xdef	_remove_ChgclrPAD

	section	.text

_remove_ChgclrPAD:
	sw	ra,.bss_1
	jal	EnterCriticalSection
	nop
	addiu	t1,zero,87
	addiu	t2,zero,176
	jalr	ra,t2
	nop
	addiu	t2,zero,9
	lw	v0,364(v0)
	nop
	addi	v1,v0,1580
.loop:
	sw	zero,0(v1)
	addiu	v1,v1,4
	addiu	t2,t2,-1
	bne	t2,zero,.loop
	nop
	jal	FlushCache
	nop
	jal	ExitCriticalSection
	nop
	lw	ra,.bss_1
	nop
	jr	ra
	nop
	nop
	nop

bss	group	bss
	section	.bss,bss
.bss_1:
	dsb	16
