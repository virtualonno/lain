#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	close
close:
	KERNEL_JUMP(0xb0, 54)
