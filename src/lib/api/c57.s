#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	InitHeap
InitHeap:
	KERNEL_JUMP(0xa0, 57)
