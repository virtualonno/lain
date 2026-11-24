#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	nextfile
nextfile:
	KERNEL_JUMP(0xb0, 67)
