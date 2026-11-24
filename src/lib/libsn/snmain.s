; LIBSN SNMAIN: the startup entry point and the global constructor and destructor runners.
	opt	c+

	xdef	__SN_ENTRY_POINT
	xdef	__main
	xdef	__do_global_dtors
	xdef	__heapbase
	xdef	__heapsize
	xdef	__text
	xdef	__textlen
	xdef	__data
	xdef	__datalen
	xdef	__bss
	xdef	__bsslen

	xref	InitHeap
	xref	main
	xref	_ramsize
	xref	_stacksize

	section	.rdata
	section	.text
	section	.ctors
	section	.dtors
	section	.data
	section	.sdata
bss	group	bss
	section	.sbss,bss
	section	.bss,bss

	section	.text

__SN_ENTRY_POINT:
	la	v0,sect(.sbss)
	la	v1,sectend(.bss)
.Lclear_bss:
	sw	zero,0(v0)
	addiu	v0,v0,4
	bltu	v0,v1,.Lclear_bss
	nop

	lw	v0,_ramsize
	nop
	addi	v0,v0,-8
	lui	t0,$8000
	or	sp,v0,t0

	la	a0,sectend(.bss)
	sll	a0,a0,3
	srl	a0,a0,3
	lw	v1,_stacksize
	nop
	subu	a1,v0,v1
	subu	a1,a1,a0
	sw	a1,__heapsize
	or	a0,a0,t0
	sw	a0,__heapbase

	sw	ra,saved_ra
	la	gp,sect(.sdata)
	move	fp,sp
	jal	InitHeap
	addi	a0,a0,4
	lw	ra,saved_ra
	nop
	jal	main
	nop
	break	1

__main:
	lw	t0,ctors_run
	addiu	sp,sp,-16
	sw	s0,4(sp)
	sw	s1,8(sp)
	sw	ra,12(sp)
	bne	t0,zero,.Lmain_done
	ori	t0,zero,1
	sw	t0,ctors_run
	la	s0,sect(.ctors)
	la	s1,(sectend(.ctors)-sect(.ctors))/4
	beq	s1,zero,.Lmain_done
	nop
.Lctor_loop:
	lw	t0,0(s0)
	addiu	s0,s0,4
	jalr	ra,t0
	addiu	s1,s1,-1
	bne	s1,zero,.Lctor_loop
	nop
.Lmain_done:
	lw	ra,12(sp)
	lw	s1,8(sp)
	lw	s0,4(sp)
	addiu	sp,sp,16
	jr	ra
	nop

__do_global_dtors:
	lw	t0,ctors_run
	addiu	sp,sp,-16
	sw	s0,4(sp)
	sw	s1,8(sp)
	sw	ra,12(sp)
	beq	t0,zero,.Ldtor_done
	nop
	la	s0,sect(.dtors)
	la	s1,(sectend(.dtors)-sect(.dtors))/4
	beq	s1,zero,.Ldtor_done
	nop
.Ldtor_loop:
	lw	t0,0(s0)
	addiu	s0,s0,4
	jalr	ra,t0
	addiu	s1,s1,-1
	bne	s1,zero,.Ldtor_loop
	nop
.Ldtor_done:
	lw	ra,12(sp)
	lw	s1,8(sp)
	lw	s0,4(sp)
	addiu	sp,sp,16
	jr	ra
	nop

	section	.data
ctors_run:
	dw	0
__heapbase:
	dw	0
__heapsize:
	dw	0
__text:
	dw	sect(.text)
__textlen:
	dw	sectend(.text)-sect(.text)
__data:
	dw	sect(.data)
__datalen:
	dw	sectend(.data)-sect(.data)
__bss:
	dw	sect(.bss)
__bsslen:
	dw	sectend(.bss)-sect(.bss)

	section	.sbss,bss
saved_ra:
	dsw	1
