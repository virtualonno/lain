#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	format
format:
	KERNEL_JUMP(0xb0, 65)
