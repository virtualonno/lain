; PlayStation BIOS kernel dispatch macros.
bios	macro	table,function
	addiu	t2,zero,\table
	jr	t2
	addiu	t1,zero,\function
	nop
	endm

bios_call	macro	table,function
	addiu	t2,zero,\table
	jalr	ra,t2
	addiu	t1,zero,\function
	endm
