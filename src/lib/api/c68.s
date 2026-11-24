#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	FlushCache
FlushCache:
	KERNEL_JUMP(0xa0, 68)
