#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	EnterCriticalSection
EnterCriticalSection:
	SYSCALL(1)
