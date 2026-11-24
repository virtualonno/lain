#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	_card_load
_card_load:
	KERNEL_JUMP(0xa0, 172)
