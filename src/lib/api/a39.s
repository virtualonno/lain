#include <asm.h>

	.set	noreorder
	.text
	.globl	SetSp
SetSp:
	addu	v0, sp, zero
	jr	ra
	addu	sp, a0, zero
	nop
