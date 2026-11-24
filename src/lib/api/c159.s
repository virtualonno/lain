#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	SetMem
SetMem:
	KERNEL_JUMP(0xa0, 159)
