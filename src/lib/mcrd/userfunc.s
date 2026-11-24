; LIBMCRD USERFUNC
	opt	c+,at-,m+

	xref	printf
	xdef	UserFuncInit
	xdef	UserFuncOpen
	xdef	UserFuncExecute
	xdef	UserFuncComplete

	section	.text

UserFuncInit:
	addiu	v0,zero,-1
	lui	at,((.data_1)+$8000)>>16
	opt	m-
	jr	ra
	sw	v0,((.data_1)-((((.data_1)+$8000)>>16)<<16))(at)
	opt	m+

UserFuncOpen:
	lw	v0,.data_1
	addiu	sp,sp,-24
	addiu	a2,v0,1
	slti	v0,a2,4
	bne	v0,zero,.user_func_open_else
	sw	ra,16(sp)
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	j	.user_func_open_loop_exit
	nop
.user_func_open_else:
	addiu	a1,zero,3
	sll	v0,a2,4
	la	v1,.bss_2
	addu	v1,v0,v1
	sll	v0,a2,2
	opt	at+
	sw	a2,.data_1
	opt	at-
	lui	at,((.bss_3)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sw	a0,((.bss_3)-((((.bss_3)+$8000)>>16)<<16))(at)
	opt	m+
.user_func_open_loop:
	sw	zero,0(v1)
	addiu	a1,a1,-1
	bgez	a1,.user_func_open_loop
	addiu	v1,v1,-4
.user_func_open_loop_exit:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

UserFuncExecute:
	lw	v1,.data_1
	addiu	sp,sp,-24
	bltz	v1,.user_func_execute_return
	sw	ra,16(sp)
	sll	v0,v1,2
	sll	v1,v1,4
	la	a0,.bss_1
	lui	at,((.bss_3)+$8000)>>16
	addu	at,at,v0
	opt	m-
	lw	v0,((.bss_3)-((((.bss_3)+$8000)>>16)<<16))(at)
	opt	m+
	nop
	jalr	ra,v0
	addu	a0,v1,a0
	beq	v0,zero,.user_func_execute_return
	nop
	lw	v0,.data_1
	nop
	addiu	v0,v0,-1
	opt	at+
	sw	v0,.data_1
	opt	at-
.user_func_execute_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

UserFuncComplete:
	lw	v0,.data_1
	jr	ra
	srl	v0,v0,31
	nop
	nop

	section	.rdata
.rdata_1:
	dw	$6d62696c
	dw	$3a647263
	dw	$65766520
	dw	$6f20746e
	dw	$66726576
	dw	$0a776f6c
	dw	$00000000
	dw	$00000000

	section	.data
.data_1:
	dw	$ffffffff
	dw	$00000000
	dw	$00000000
	dw	$00000000

bss	group	bss
	section	.bss,bss
.bss_1:
	dsb	12
.bss_2:
	dsb	52
.bss_3:
	dsb	16
