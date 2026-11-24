#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	ResetEntryInt
ResetEntryInt:
	KERNEL_JUMP(0xb0, 24)
