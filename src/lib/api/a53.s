#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	write
write:
	KERNEL_JUMP(0xb0, 53)
