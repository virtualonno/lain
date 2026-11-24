#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	firstfile2
firstfile2:
	KERNEL_JUMP(0xb0, 66)
