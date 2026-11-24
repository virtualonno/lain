#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	SysDeqIntRP
SysDeqIntRP:
	KERNEL_JUMP(0xc0, 3)
