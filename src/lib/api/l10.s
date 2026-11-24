#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	ChangeClearRCnt
ChangeClearRCnt:
	KERNEL_JUMP(0xc0, 10)
