#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	SysEnqIntRP
SysEnqIntRP:
	KERNEL_JUMP(0xc0, 2)
