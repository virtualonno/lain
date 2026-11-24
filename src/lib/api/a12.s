#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	EnableEvent
EnableEvent:
	KERNEL_JUMP(0xb0, 12)
