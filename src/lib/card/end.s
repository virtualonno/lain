#include <asm.h>
#include <kernel_call.h>

	.text
	.set	noreorder
	.globl	_ExitCard
_ExitCard:
	.set	noat
	lui	$1, %hi(saved_ra)
	sw	$31, %lo(saved_ra)($1)
	.set	at
	jal	EnterCriticalSection
	nop
	addiu	$9, $0, 86
	addiu	$10, $0, 176
	jalr	$31, $10
	nop
	lw	$2, 24($2)
	lui	$10, %hi(.Lpatch)
	addiu	$10, $10, %lo(.Lpatch)
	lui	$9, %hi(.Lpatch_end)
	addiu	$9, $9, %lo(.Lpatch_end)
.Lcopy:
	lw	$3, 0($10)
	nop
	sw	$3, 112($2)
	addiu	$10, $10, 4
	bne	$10, $9, .Lcopy
	addiu	$2, $2, 4
	jal	FlushCache
	nop
	jal	ExitCriticalSection
	nop
	lui	$31, %hi(saved_ra)
	lw	$31, %lo(saved_ra)($31)
	nop
	jr	$31
	nop
.Lpatch:
	nop
	nop
	nop
.Lpatch_end:
	nop

	.bss
	.align	3
saved_ra:
	.space	16
