#include <asm.h>
#include <kernel_call.h>

	.set	noreorder
	.text
	.globl	ExitCriticalSection
ExitCriticalSection:
	SYSCALL(2)
