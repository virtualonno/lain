#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	StopPAD2
StopPAD2:
	KERNEL_JUMP(0xb0, 20)
