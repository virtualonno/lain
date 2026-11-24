; LIBC2 PUTCHAR
	opt	c+,at-,m+

	xref	_ctype_
	xref	write
	xdef	_putchar
	xdef	_putchar_flash
	xdef	putchar

	section	.text

_putchar:
	addiu	sp,sp,-24
	sw	s0,16(sp)
	addu	s0,a0,zero
	sll	a0,a0,24
	sra	a0,a0,24
	addiu	v0,zero,9
	beq	a0,v0,.putchar_loop
	sw	ra,20(sp)
	addiu	v0,zero,10
	bne	a0,v0,.putchar_loop_exit
	andi	v0,s0,$ff
	jal	_putchar
	addiu	a0,zero,13
	lui	at,((.data_1)+$8000)>>16
	opt	m-
	j	.putchar_join
	sw	zero,((.data_1)-((((.data_1)+$8000)>>16)<<16))(at)
	opt	m+
.putchar_loop:
	jal	_putchar
	addiu	a0,zero,32
	lw	v0,.data_1
	nop
	andi	v0,v0,$7
	beq	v0,zero,.putchar_loop_exit_2
	nop
	j	.putchar_loop
	nop
.putchar_loop_exit:
	lui	at,((_ctype_+$1)+$8000)>>16
	addu	at,at,v0
	opt	m-
	lb	v0,((_ctype_+$1)-((((_ctype_+$1)+$8000)>>16)<<16))(at)
	opt	m+
	nop
	andi	v0,v0,$97
	beq	v0,zero,.putchar_join
	nop
	lw	v0,.data_1
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.data_1
	opt	at-
.putchar_join:
	lw	a2,.data_2
	nop
	slti	v0,a2,32
	bne	v0,zero,.putchar_return
	nop
	la	a1,.bss_1
	jal	write
	addiu	a0,zero,1
	opt	at+
	sw	zero,.data_2
	opt	at-
.putchar_return:
	la	v1,.data_2
	lw	v0,0(v1)
	lui	at,((.bss_1)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sb	s0,((.bss_1)-((((.bss_1)+$8000)>>16)<<16))(at)
	opt	m+
	addiu	v0,v0,1
	sw	v0,0(v1)
.putchar_loop_exit_2:
	lw	ra,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,24

_putchar_flash:
	lw	a2,.data_2
	addiu	sp,sp,-24
	blez	a2,.putchar_flash_return
	sw	ra,16(sp)
	la	a1,.bss_1
	jal	write
	addiu	a0,zero,1
	opt	at+
	sw	zero,.data_2
	opt	at-
.putchar_flash_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

putchar:
	addiu	sp,sp,-24
	sw	s0,16(sp)
	addu	s0,a0,zero
	sll	a0,a0,24
	sra	a0,a0,24
	addiu	v0,zero,9
	beq	a0,v0,.putchar_loop_2
	sw	ra,20(sp)
	addiu	v0,zero,10
	bne	a0,v0,.putchar_loop_exit_3
	andi	v0,s0,$ff
	jal	_putchar
	addiu	a0,zero,13
	lui	at,((.data_1)+$8000)>>16
	opt	m-
	j	.putchar_join_2
	sw	zero,((.data_1)-((((.data_1)+$8000)>>16)<<16))(at)
	opt	m+
.putchar_loop_2:
	jal	_putchar
	addiu	a0,zero,32
	lw	v0,.data_1
	nop
	andi	v0,v0,$7
	beq	v0,zero,.putchar_loop_exit_4
	nop
	j	.putchar_loop_2
	nop
.putchar_loop_exit_3:
	lui	at,((_ctype_+$1)+$8000)>>16
	addu	at,at,v0
	opt	m-
	lb	v0,((_ctype_+$1)-((((_ctype_+$1)+$8000)>>16)<<16))(at)
	opt	m+
	nop
	andi	v0,v0,$97
	beq	v0,zero,.putchar_join_2
	nop
	lw	v0,.data_1
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.data_1
	opt	at-
.putchar_join_2:
	lw	a2,.data_2
	nop
	slti	v0,a2,32
	bne	v0,zero,.putchar_skip
	nop
	la	a1,.bss_1
	jal	write
	addiu	a0,zero,1
	opt	at+
	sw	zero,.data_2
	opt	at-
.putchar_skip:
	lw	v0,.data_2
	lui	at,((.bss_1)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sb	s0,((.bss_1)-((((.bss_1)+$8000)>>16)<<16))(at)
	opt	m+
	addiu	v0,v0,1
	opt	at+
	sw	v0,.data_2
	opt	at-
.putchar_loop_exit_4:
	lw	a2,.data_2
	nop
	blez	a2,.putchar_return_2
	nop
	la	a1,.bss_1
	jal	write
	addiu	a0,zero,1
	opt	at+
	sw	zero,.data_2
	opt	at-
.putchar_return_2:
	lw	ra,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,24

	section	.data
.data_1:
	dw	$00000000
.data_2:
	dw	$00000000
	dw	$00000000
	dw	$00000000

bss	group	bss
	section	.bss,bss
.bss_1:
	dsb	32
