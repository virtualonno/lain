#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	StopCARD2
StopCARD2:
	KERNEL_JUMP(0xb0, 76)
