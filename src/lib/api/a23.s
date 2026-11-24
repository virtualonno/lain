#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	ReturnFromException
ReturnFromException:
	KERNEL_JUMP(0xb0, 23)
