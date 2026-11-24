	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>

	xref	SQRT
	xdef	SquareRoot12

	section	.text

SquareRoot12:
	mtc2	a0,C2_LZCS
	nop
	nop
	mfc2	v0,C2_LZCR
	addiu	at,zero,32
	beq	v0,at,.Lzero
	nop
	andi	t0,v0,$1
	addiu	t2,zero,-2
	and	t2,v0,t2
	addiu	t1,zero,19
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
	bltz	t1,.Lscale_down
	nop
	sllv	v0,t5,t1
	jr	ra
	nop
.Lscale_down:
	sub	t1,zero,t1
	jr	ra
	srlv	v0,t5,t1
.Lzero:
	jr	ra
	addiu	v0,zero,0
	nop
	nop
	nop
