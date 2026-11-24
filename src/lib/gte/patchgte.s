	opt	c+
#include <hilo.h>
#include <kernel_call.h>
#include <r3000.h>

	xref	EnterCriticalSection
	xref	FlushCache
	xref	ExitCriticalSection
	xdef	_patch_gte

	section	.text

_patch_gte:
	sw	ra,save_ra
	jal	EnterCriticalSection
	nop
	addiu	t1,zero,GetC0Table
	addiu	t2,zero,$b0
	jalr	ra,t2
	nop
	lw	v0,C0TBL_EXCEPTION(v0)
	nop
	addiu	v0,v0,40
	addu	t7,v0,zero
	lui	t2,HI(.expected)
	addiu	t2,t2,LO(.expected)
	lui	t1,HI(.replacement)
	addiu	t1,t1,LO(.replacement)

	; Install the GTE fix only on the kernel handler it was written for.
.check_handler:
	lw	v1,0(t2)
	lw	t3,0(v0)
	addiu	t2,t2,4
	bne	v1,t3,.finish
	addiu	v0,v0,4
	bne	t2,t1,.check_handler
	nop
	addu	v0,t7,zero
	lui	t2,HI(.replacement)
	addiu	t2,t2,LO(.replacement)
	lui	t1,HI(.replacement_end)
	addiu	t1,t1,LO(.replacement_end)
.copy_handler:
	lw	v1,0(t2)
	nop
	sw	v1,0(v0)
	addiu	t2,t2,4
	bne	t2,t1,.copy_handler
	addiu	v0,v0,4
.finish:
	jal	FlushCache
	nop
	jal	ExitCriticalSection
	nop
	lw	ra,save_ra
	nop
	jr	ra
	nop

.expected:
	sw	at,4(k0)
	sw	v0,8(k0)
	sw	v1,12(k0)
	sw	ra,124(k0)
	mfc0	v1,epc
	nop
.replacement:
	sw	at,4(k0)
	sw	v0,8(k0)
	mfc0	v0,cause
	sw	v1,12(k0)
	mfc0	v1,epc
	sw	ra,124(k0)
.replacement_end:
	nop

bss	group	bss
	section	.bss,bss
save_ra:
	dsw	1
	dsw	3
