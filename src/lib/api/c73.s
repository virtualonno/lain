#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	GPU_cw
GPU_cw:
	KERNEL_JUMP(0xa0, 73)
