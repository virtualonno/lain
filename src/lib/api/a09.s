#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	CloseEvent
CloseEvent:
	KERNEL_JUMP(0xb0, 9)
