#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	AddDrv
AddDrv:
	KERNEL_JUMP(0xb0, 71)
