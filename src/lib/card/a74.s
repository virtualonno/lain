#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	InitCARD2
InitCARD2:
	KERNEL_JUMP(0xb0, 74)
