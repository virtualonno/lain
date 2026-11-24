#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	InitPAD2
InitPAD2:
	KERNEL_JUMP(0xb0, 18)
