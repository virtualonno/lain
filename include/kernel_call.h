/* Calls into the kernel: a dispatcher at 0xa0, 0xb0 or 0xc0 takes the function's number in t1. */
#ifndef _KERNEL_CALL_H
#define _KERNEL_CALL_H

#define KERNEL_JUMP(table, function) addiu t2, zero, table; jr t2; addiu t1, zero, function; nop
#define SYSCALL(number) addiu a0, zero, number; syscall 0; jr ra; nop

#ifndef _ASM_H
/* A kernel call that returns to its caller, where KERNEL_JUMP leaves for good */
KCALL	macro	table,function
	addiu	t2,zero,\table
	jalr	ra,t2
	addiu	t1,zero,\function
	endm
#endif

#define GetC0Table	86
#define GetB0Table	87

/* entries of those tables, as byte offsets */
#define C0TBL_EXCEPTION		(6*4)
#define B0TBL_CHANGECLEARPAD	(91*4)

#endif /* _KERNEL_CALL_H */
