#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	WaitEvent
WaitEvent:
	KERNEL_JUMP(0xb0, 10)
