#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	_card_info
_card_info:
	KERNEL_JUMP(0xa0, 171)
