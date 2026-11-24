#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	DisableEvent
DisableEvent:
	KERNEL_JUMP(0xb0, 13)
