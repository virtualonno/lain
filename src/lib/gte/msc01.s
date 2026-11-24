	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>

	xref	SQRT
	xdef	SquareRoot0

	section	.text

SquareRoot0:
	mtc2	a0,C2_LZCS
	nop
	nop
	mfc2	v0,C2_LZCR
	addiu	at,zero,32
	beq	v0,at,.Lzero
	nop
	andi	t0,v0,$1
	addiu	at,zero,-2
	and	t2,v0,at
	addiu	t1,zero,31
	sub	t1,t1,t2
	sra	t1,t1,1
	addi	t3,t2,-24
	bltz	t3,.Lshr
	nop
	sllv	t4,a0,t3
	beq	zero,zero,.Llookup
.Lshr:
	addiu	t3,zero,24
	sub	t3,t3,t2
	srav	t4,a0,t3
.Llookup:
	addi	t4,t4,-64
	sll	t4,t4,1
	lui	t5,HI(SQRT)
	addu	t5,t5,t4
	lh	t5,LO(SQRT)(t5)
	nop
	sllv	t5,t5,t1
	srl	v0,t5,12
	jr	ra
	nop
.Lzero:
	jr	ra
	addiu	v0,zero,0
	nop
	nop
	nop
