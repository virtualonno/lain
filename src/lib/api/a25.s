#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	HookEntryInt
HookEntryInt:
	KERNEL_JUMP(0xb0, 25)
