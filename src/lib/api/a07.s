#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	DeliverEvent
DeliverEvent:
	KERNEL_JUMP(0xb0, 7)
