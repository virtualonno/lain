#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	_card_write
_card_write:
	KERNEL_JUMP(0xb0, 78)
