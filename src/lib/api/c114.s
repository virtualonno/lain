#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.word	0x25007350, 0x0043529b	/* Eight unreferenced bytes before the entry. */
	.globl	_96_remove
_96_remove:
	KERNEL_JUMP(0xa0, 114)
	nop
	nop
