#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	StartPAD2
StartPAD2:
	KERNEL_JUMP(0xb0, 19)
