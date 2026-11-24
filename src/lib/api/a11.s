#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	TestEvent
TestEvent:
	KERNEL_JUMP(0xb0, 11)
