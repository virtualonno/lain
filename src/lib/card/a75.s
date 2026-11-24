#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	StartCARD2
StartCARD2:
	KERNEL_JUMP(0xb0, 75)
