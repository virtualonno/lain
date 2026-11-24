#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	ChangeClearPAD
ChangeClearPAD:
	KERNEL_JUMP(0xb0, 91)
