#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	DelDrv
DelDrv:
	KERNEL_JUMP(0xb0, 72)
