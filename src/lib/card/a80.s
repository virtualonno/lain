#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	_new_card
_new_card:
	KERNEL_JUMP(0xb0, 80)
