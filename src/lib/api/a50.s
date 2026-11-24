#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	open
open:
	KERNEL_JUMP(0xb0, 50)
