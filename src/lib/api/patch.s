; Redirect the controller enable and disable entries and clear the BIOS reset sequence.
	opt	c+
#include <hilo.h>
#include <kernel_call.h>

	xref	EnterCriticalSection
	xref	FlushCache
	xdef	EnablePAD
	xdef	DisablePAD
	xdef	_patch_pad

	section	.text

; Enable or disable controller polling through the installed BIOS entry.
EnablePAD:
	lw	t1,enable_pad
	nop
	jr	t1
	nop
DisablePAD:
	lw	t1,disable_pad
	nop
	jr	t1
	nop

_patch_pad:
	sw	ra,saved_ra
	jal	EnterCriticalSection
	nop
	opt	m-
	addiu	t1,zero,GetB0Table
	addiu	t2,zero,$b0
	jalr	t2
	nop
	lw	v0,B0TBL_CHANGECLEARPAD(v0)
	addiu	t1,zero,11
	addi	v1,v0,$884
	lui	at,HI(enable_pad)
	sw	v1,LO(enable_pad)(at)
	addi	v1,v0,$894
	lui	at,HI(disable_pad)
	sw	v1,LO(disable_pad)(at)
	opt	m+
.clear:
	sw	zero,$594(v0)
	addiu	v0,v0,4
	addiu	t1,t1,-1
	bne	t1,zero,.clear
	nop
	jal	FlushCache
	nop
	lw	ra,saved_ra
	nop
	jr	ra
	nop
	nop
	nop

bss	group	bss
	section	.bss,bss
saved_ra:
	dsw	1
unused:			; Four unreferenced bytes.
	dsw	1
enable_pad:
	dsw	1
disable_pad:
	dsw	1
