#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	OpenEvent
OpenEvent:
	KERNEL_JUMP(0xb0, 8)
