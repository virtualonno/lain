#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	_bu_init
_bu_init:
	KERNEL_JUMP(0xa0, 112)
