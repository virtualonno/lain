#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	read
read:
	KERNEL_JUMP(0xb0, 52)
