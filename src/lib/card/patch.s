#include <asm.h>
#include <kernel_call.h>

#define PATCH_RAM        0xdf80
#define RESUME_PC_RAM    0xdffc
#define RAM_HI           0x0001
#define UNCACHED_RAM_HI  0xa001
#define RAM_LO(addr)     ((addr) - 0x10000)

#define IRQ_MASK_OFFSET    0x1074
#define SIO0_STATUS_OFFSET 0x1044
#define PAD_MEMCARD_IRQ    0x80
#define SIO0_STATUS_ACK    0x80

	.text
	.set	noreorder

/* Copied to PATCH_RAM before installing either BIOS hook. The fragments run
 * with the BIOS's registers, not the C calling convention. */
.Lram_patch:
	lhu	t7, 10(v1)
	lui	t0, 0
	or	t8, t7, v0
	ori	t9, t8, 0x12
	sh	t9, 10(v1)
	addiu	t0, zero, 40
.Lcontrol_delay:
	addiu	t0, t0, -1
	bne	t0, zero, .Lcontrol_delay
	nop
	jr	ra
	nop

.Lirq_hook:
	lw	v0, IRQ_MASK_OFFSET(v1)
	nop
	andi	v0, v0, PAD_MEMCARD_IRQ
	beq	v0, zero, .Lirq_done
	nop
.Lwait_ack:
	lw	v0, SIO0_STATUS_OFFSET(v1)
	nop
	andi	v0, v0, SIO0_STATUS_ACK
	bne	v0, zero, .Lwait_ack
	nop
	lui	v0, RAM_HI
	lw	v0, RAM_LO(RESUME_PC_RAM)(v0)
	nop
	jr	v0
	nop
.Lirq_done:
	jr	ra
	nop
.Lram_patch_end:

/* The trampolines target the uncached RAM copy, not this text section. */
.Lexception_jump:
	lui	v0, UNCACHED_RAM_HI
	addiu	v0, v0, RAM_LO(PATCH_RAM + (.Lirq_hook - .Lram_patch))
	jr	v0
	nop
	nop
.Lclear_pad_call:
	lui	t0, UNCACHED_RAM_HI
	addiu	t0, t0, RAM_LO(PATCH_RAM)
	jalr	ra, t0
	nop
	nop
.Lclear_pad_call_end:

	.globl	_patch_card
_patch_card:
	.set	noat
	lui	AT, %hi(saved_ra)
	sw	ra, %lo(saved_ra)(AT)
	.set	at
	jal	EnterCriticalSection
	nop
	addiu	t1, zero, GetC0Table
	addiu	t2, zero, 0xb0
	jalr	ra, t2
	nop
	lw	v0, C0TBL_EXCEPTION(v0)
	nop
	/* The exception entry encodes the two halves of the handler address. */
	lw	v1, 0x70(v0)
	nop
	andi	t1, v1, 0xffff
	sll	t1, t1, 16
	lw	v1, 0x74(v0)
	nop
	andi	t2, v1, 0xffff
	addu	v1, t1, t2
	addiu	v0, v1, 0x28
	lui	t2, %hi(.Lexception_jump)
	addiu	t2, t2, %lo(.Lexception_jump)
	lui	t1, %hi(.Lclear_pad_call)
	addiu	t1, t1, %lo(.Lclear_pad_call)
.Linstall_exception_jump:
	lw	v1, 0(t2)
	nop
	sw	v1, 0(v0)
	addiu	t2, t2, 4
	bne	t2, t1, .Linstall_exception_jump
	addiu	v0, v0, 4
	.set	noat
	lui	AT, RAM_HI
	jal	FlushCache
	sw	v0, RAM_LO(RESUME_PC_RAM)(AT)
	.set	at
	lui	ra, %hi(saved_ra)
	lw	ra, %lo(saved_ra)(ra)
	nop
	jr	ra
	nop

	.globl	_patch_card2
_patch_card2:
	.set	noat
	lui	AT, %hi(saved_ra)
	sw	ra, %lo(saved_ra)(AT)
	.set	at
	jal	EnterCriticalSection
	nop
	addiu	t1, zero, GetB0Table
	addiu	t2, zero, 0xb0
	jalr	ra, t2
	nop
	lw	v0, B0TBL_CHANGECLEARPAD(v0)
	nop
	lw	v1, 0x9c8(v0)
	lui	t2, %hi(.Lclear_pad_call)
	addiu	t2, t2, %lo(.Lclear_pad_call)
	lui	t1, %hi(.Lclear_pad_call_end)
	addiu	t1, t1, %lo(.Lclear_pad_call_end)
.Linstall_clear_pad_call:
	lw	t0, 0(t2)
	nop
	sw	t0, 0x9c8(v0)
	addiu	t2, t2, 4
	bne	t2, t1, .Linstall_clear_pad_call
	addiu	v0, v0, 4
	jal	FlushCache
	nop
	lui	ra, %hi(saved_ra)
	lw	ra, %lo(saved_ra)(ra)
	nop
	jr	ra
	nop

	.globl	_copy_memcard_patch
_copy_memcard_patch:
	ori	v0, zero, PATCH_RAM
	lui	t2, %hi(.Lram_patch)
	addiu	t2, t2, %lo(.Lram_patch)
	lui	t1, %hi(.Lram_patch_end)
	addiu	t1, t1, %lo(.Lram_patch_end)
.Lcopy_ram_patch:
	lw	v1, 0(t2)
	nop
	sw	v1, 0(v0)
	addiu	t2, t2, 4
	bne	t2, t1, .Lcopy_ram_patch
	addiu	v0, v0, 4
	jr	ra
	nop

	.bss
	.align	3
saved_ra:
	.space	16
