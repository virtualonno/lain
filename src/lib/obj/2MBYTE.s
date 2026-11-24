; 2MBYTE: the Runtime Library's startup for a 2 MB machine, linked first. It clears .sbss and .bss,
; puts the stack at the top of RAM less 8 and gp at .sdata, gives InitHeap the memory between .bss and
; the stack, then calls main. __main, which main calls first, does nothing here.
	opt	c+

	xdef	__main
	xdef	__SN_ENTRY_POINT
	xdef	stup0
	xdef	stup1
	xdef	stup2

	xref	InitHeap
	xref	main
	xref	_stacksize

	section	.rdata
	section	.text
	section	.data
	section	.sdata
bss	group	bss
	section	.sbss,bss
	section	.bss,bss

	section	.text

__main:
	jr	ra
	nop

__SN_ENTRY_POINT:
stup2:
	la	v0,sect(.sbss)
	la	v1,sectend(.bss)
.Lclear_bss:
	sw	zero,0(v0)
	addiu	v0,v0,4
	bltu	v0,v1,.Lclear_bss
	nop

stup1:
	li	v0,4
	nop
	nop
	nop
	nop
	la	a0,ram_sizes
	addu	a0,a0,v0
	lw	v0,0(a0)
	lui	t0,$8000
	or	sp,v0,t0

	la	a0,sectend(.bss)
	sll	a0,a0,3
	srl	a0,a0,3
	lw	v1,_stacksize
	nop
	subu	a1,v0,v1
	subu	a1,a1,a0
	or	a0,a0,t0

	sw	ra,saved_ra
	la	gp,sect(.sdata)
	move	fp,sp
	jal	InitHeap
	addi	a0,a0,4
	lw	ra,saved_ra
	nop

stup0:
	jal	main
	nop
	break	1

; Bytes of RAM; the word at offset 4 is the one read.
ram_sizes:
	dw	$00200000
	dw	$00200000
	dw	$00200000
	dw	$00200000

	section	.sbss,bss
saved_ra:
	dsw	1
