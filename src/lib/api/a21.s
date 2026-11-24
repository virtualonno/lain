#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	PAD_init2
PAD_init2:
	KERNEL_JUMP(0xb0, 21)
