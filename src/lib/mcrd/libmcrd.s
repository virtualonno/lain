; LIBMCRD LIBMCRD
	opt	c+,at-,m+

	xref	UserFuncComplete
	xref	UserFuncExecute
	xref	UserFuncInit
	xref	UserFuncOpen
	xref	VSyncCallbacks
	xref	_card_clear
	xref	_card_close
	xref	_card_info
	xref	_card_load
	xref	_card_open
	xref	_card_start
	xref	_card_stop
	xref	_card_write
	xref	_chk_card_event
	xref	_chk_card_event_x
	xref	_clr_card_event
	xref	_get_card_event
	xref	_get_card_event_x
	xref	_new_card
	xref	close
	xref	erase
	xref	firstfile
	xref	format
	xref	lseek
	xref	nextfile
	xref	open
	xref	printf
	xref	read
	xref	strcat
	xref	write
	xdef	MemCardInit
	xdef	MemCardEnd
	xdef	MemCardStart
	xdef	MemCardStop
	xdef	MemCardExist
	xdef	MemCardAccept
	xdef	MemCardOpen
	xdef	MemCardClose
	xdef	MemCardReadData
	xdef	MemCardWriteData
	xdef	MemCardReadFile
	xdef	MemCardWriteFile
	xdef	MemCardGetDirentry
	xdef	MemCardCallback
	xdef	MemCardSync
	xdef	MemCardCreateFile
	xdef	MemCardDeleteFile
	xdef	MemCardFormat
	xdef	MemCardUnformat

	section	.text

MemCardInit:
	addiu	sp,sp,-24
	sw	ra,16(sp)
	opt	at+
	sw	zero,.bss_6
	opt	at-
	lui	at,((.bss_12)+$8000)>>16
	opt	m-
	jal	_card_open
	sw	zero,((.bss_12)-((((.bss_12)+$8000)>>16)<<16))(at)
	opt	m+
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

MemCardEnd:
	addiu	sp,sp,-24
	sw	ra,16(sp)
	jal	_card_close
	nop
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

MemCardStart:
	addiu	sp,sp,-24
	sw	ra,16(sp)
	jal	UserFuncInit
	nop
	la	v0,.bss_7
	addiu	v1,zero,-1
	sw	zero,0(v0)
	sw	zero,4(v0)
	sw	zero,8(v0)
	jal	_card_start
	sw	v1,16(v0)
	la	a1,.mem_card_unformat_block
	jal	VSyncCallbacks
	addiu	a0,zero,7
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

MemCardStop:
	addiu	sp,sp,-24
	la	v1,.bss_7
	sw	ra,16(sp)
.mem_card_stop_loop:
	lw	v0,0(v1)
	nop
	bne	v0,zero,.mem_card_stop_loop
	addiu	a0,zero,7
	jal	VSyncCallbacks
	addu	a1,zero,zero
	jal	_card_stop
	nop
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

MemCardExist:
	addiu	sp,sp,-24
	la	v1,.bss_7
	sw	ra,16(sp)
	lw	v0,0(v1)
	nop
	bgtz	v0,.mem_card_exist_else
	addu	a1,a0,zero
	la	a0,.mem_card_exist_block
	addiu	v0,zero,1
	sw	v0,0(v1)
	sw	zero,4(v1)
	sw	zero,8(v1)
	jal	UserFuncOpen
	sw	a1,12(v1)
	j	.mem_card_exist_return
	addiu	v0,zero,1
.mem_card_exist_else:
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	addu	v0,zero,zero
.mem_card_exist_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
.mem_card_exist_block:
	addiu	sp,sp,-32
	sw	s0,16(sp)
	addu	s0,a0,zero
	sw	ra,24(sp)
	sw	s1,20(sp)
	lw	v1,0(s0)
	nop
	beq	v1,zero,.mem_card_exist_else_2
	addiu	v0,zero,10
	bltz	v1,.mem_card_exist_return_2
	addiu	s1,zero,10
	beq	v1,s1,.mem_card_exist_skip
	addiu	v0,zero,11
	beq	v1,v0,.mem_card_exist_else_3
	addu	v0,zero,zero
	j	.mem_card_exist_return_3
	nop
.mem_card_exist_else_2:
	opt	at+
	sw	zero,.bss_2
	sw	zero,.bss_1
	opt	at-
	sw	v0,0(s0)
.mem_card_exist_skip:
	jal	_clr_card_event
	nop
	lw	a0,.bss_10
	jal	_card_info
	nop
	lw	v0,0(s0)
	nop
	addiu	v0,v0,1
	j	.mem_card_exist_return_2
	sw	v0,0(s0)
.mem_card_exist_else_3:
	jal	_chk_card_event
	nop
	beq	v0,zero,.mem_card_exist_return_3
	addu	v0,zero,zero
	jal	_get_card_event
	nop
	addu	v1,v0,zero
	opt	at+
	sw	v0,.bss_2
	opt	at-
	slti	v0,v1,3
	beq	v0,zero,.mem_card_exist_else_4
	nop
	bgtz	v1,.mem_card_exist_else_6
	nop
	beq	v1,zero,.mem_card_exist_else_5
	addiu	v0,zero,1
	j	.mem_card_exist_else_7
	nop
.mem_card_exist_else_4:
	addiu	v0,zero,4
	bne	v1,v0,.mem_card_exist_else_7
	nop
	jal	.mem_card_unformat_helper
	addiu	a0,zero,4
	la	v1,.bss_7
	sw	v0,4(v1)
	j	.mem_card_exist_return_3
	addiu	v0,zero,1
.mem_card_exist_else_5:
	la	s0,.bss_10
	lw	a0,0(s0)
	lw	v1,.bss_6
	sllv	v0,v0,a0
	and	v1,v1,v0
	bne	v1,zero,.mem_card_exist_skip_2
	addiu	v0,zero,4
	opt	at+
	sw	v0,.bss_2
	opt	at-
.mem_card_exist_skip_2:
	lw	a0,.bss_2
	jal	.mem_card_unformat_helper
	nop
	addu	a0,v0,zero
	addiu	v0,zero,1
	addiu	v1,s0,-12
	j	.mem_card_exist_return_3
	sw	a0,4(v1)
.mem_card_exist_else_6:
	lw	v0,.bss_1
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.bss_1
	opt	at-
	slti	v0,v0,5
	beq	v0,zero,.mem_card_exist_else_7
	nop
	j	.mem_card_exist_return_2
	sw	s1,0(s0)
.mem_card_exist_else_7:
	la	s0,.bss_10
	addiu	v0,zero,1
	lw	v1,0(s0)
	lw	a0,.bss_2
	sllv	v0,v0,v1
	lw	v1,.bss_6
	nor	v0,zero,v0
	and	v1,v1,v0
	opt	at+
	sw	v1,.bss_6
	opt	at-
	jal	.mem_card_unformat_helper
	addiu	s0,s0,-12
	addu	v1,v0,zero
	addiu	v0,zero,1
	j	.mem_card_exist_return_3
	sw	v1,4(s0)
.mem_card_exist_return_2:
	addu	v0,zero,zero
.mem_card_exist_return_3:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardAccept:
	addiu	sp,sp,-24
	la	v1,.bss_7
	sw	ra,16(sp)
	lw	v0,0(v1)
	nop
	bgtz	v0,.mem_card_accept_else
	addu	a1,a0,zero
	la	a0,.mem_card_accept_block
	addiu	v0,zero,2
	sw	v0,0(v1)
	sw	zero,4(v1)
	sw	zero,8(v1)
	jal	UserFuncOpen
	sw	a1,12(v1)
	j	.mem_card_accept_return
	addiu	v0,zero,1
.mem_card_accept_else:
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	addu	v0,zero,zero
.mem_card_accept_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
.mem_card_accept_block:
	addiu	sp,sp,-32
	sw	s0,16(sp)
	addu	s0,a0,zero
	sw	ra,24(sp)
	sw	s1,20(sp)
	lw	v1,0(s0)
	nop
	sltiu	v0,v1,32
	beq	v0,zero,.mem_card_accept_case_default
	sll	v0,v1,2
	lui	at,((.rdata_2)+$8000)>>16
	addu	at,at,v0
	opt	m-
	lw	v0,((.rdata_2)-((((.rdata_2)+$8000)>>16)<<16))(at)
	opt	m+
	nop
	jr	v0
	nop
.mem_card_accept_case_0:
	la	a0,.mem_card_exist_block
	opt	at+
	sw	zero,.bss_5
	sw	zero,.bss_4
	opt	at-
	lui	at,((.bss_3)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	zero,((.bss_3)-((((.bss_3)+$8000)>>16)<<16))(at)
	opt	m+
	addiu	v0,zero,10
	j	.mem_card_accept_case_default
	sw	v0,0(s0)
.mem_card_accept_case_10:
	la	s1,.bss_8
	lw	v1,0(s1)
	nop
	beq	v1,zero,.mem_card_accept_skip_2
	addiu	v0,zero,3
	bne	v1,v0,.mem_card_accept_return_2
	addiu	v0,zero,1
	lw	a0,8(s1)
	lw	v0,.bss_6
	addiu	v1,zero,1
	opt	at+
	sw	v1,.bss_5
	opt	at-
	sllv	v1,v1,a0
	or	v0,v0,v1
	lui	at,((.bss_6)+$8000)>>16
	opt	m-
	jal	_clr_card_event
	sw	v0,((.bss_6)-((((.bss_6)+$8000)>>16)<<16))(at)
	opt	m+
	lw	a0,8(s1)
	jal	_card_clear
	nop
	addiu	v0,zero,21
	j	.mem_card_accept_case_default
	sw	v0,0(s0)
.mem_card_accept_case_21:
	jal	_chk_card_event_x
	nop
	beq	v0,zero,.mem_card_accept_return_2
	addu	v0,zero,zero
	jal	_get_card_event_x
	nop
	addiu	v0,zero,30
	sw	v0,0(s0)
.mem_card_accept_case_30:
	jal	_clr_card_event
	nop
	lw	a0,.bss_10
	jal	_card_load
	nop
	lw	v0,0(s0)
	nop
	addiu	v0,v0,1
	j	.mem_card_accept_case_default
	sw	v0,0(s0)
.mem_card_accept_case_31:
	jal	_chk_card_event
	nop
	beq	v0,zero,.mem_card_accept_return_2
	addu	v0,zero,zero
	jal	_get_card_event
	nop
	addu	v1,v0,zero
	opt	at+
	sw	v0,.bss_4
	opt	at-
	slti	v0,v1,3
	beq	v0,zero,.mem_card_accept_else_2
	nop
	bgtz	v1,.mem_card_accept_else_4
	nop
	beq	v1,zero,.mem_card_accept_else_3
	nop
	j	.mem_card_accept_else_5
	nop
.mem_card_accept_else_2:
	addiu	v0,zero,4
	beq	v1,v0,.mem_card_accept_else_4
	nop
	j	.mem_card_accept_else_5
	nop
.mem_card_accept_else_3:
	lw	v0,.bss_5
	nop
	beq	v0,zero,.mem_card_accept_skip
	addu	v1,zero,zero
	addiu	v1,zero,3
.mem_card_accept_skip:
	la	v0,.bss_7
	sw	v1,4(v0)
	j	.mem_card_accept_return_2
	addiu	v0,zero,1
.mem_card_accept_else_4:
	lw	v0,.bss_3
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.bss_3
	opt	at-
	slti	v0,v0,5
	beq	v0,zero,.mem_card_accept_else_5
	nop
.mem_card_accept_skip_2:
	addiu	v0,zero,30
	j	.mem_card_accept_case_default
	sw	v0,0(s0)
.mem_card_accept_else_5:
	lw	a0,.bss_4
	addiu	v0,zero,4
	bne	a0,v0,.mem_card_accept_else_6
	nop
	la	v0,.bss_7
	sw	a0,4(v0)
	j	.mem_card_accept_return_2
	addiu	v0,zero,1
.mem_card_accept_else_6:
	jal	.mem_card_unformat_helper
	nop
	la	v1,.bss_7
	sw	v0,4(v1)
	j	.mem_card_accept_return_2
	addiu	v0,zero,1
.mem_card_accept_case_default:
	addu	v0,zero,zero
.mem_card_accept_return_2:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardOpen:
	addiu	sp,sp,-64
	sw	s3,36(sp)
	addu	s3,a0,zero
	sw	s4,40(sp)
	addu	s4,a1,zero
	sw	s5,44(sp)
	addu	s5,a2,zero
	sw	s1,28(sp)
	la	s1,.bss_11
	sw	ra,56(sp)
	sw	s7,52(sp)
	sw	s6,48(sp)
	sw	s2,32(sp)
	sw	s0,24(sp)
	lw	v0,0(s1)
	nop
	bltz	v0,.mem_card_open_else
	addu	s2,zero,zero
	lui	a0,((.rdata_3)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_3)-((((.rdata_3)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_open_return
	addiu	v0,zero,-1
.mem_card_open_else:
	addu	a0,s3,zero
	addiu	s0,s1,16
	jal	.mem_card_unformat_helper_2
	addu	a1,s0,zero
	addu	a0,s0,zero
	jal	strcat
	addu	a1,s4,zero
	sw	s3,-4(s1)
	addiu	s4,s1,-16
	addiu	s1,zero,2
	addiu	s6,zero,3
	addiu	s7,zero,5
.mem_card_open_loop:
	jal	_clr_card_event
	nop
	addu	a0,s0,zero
	jal	open
	ori	a1,s5,$8000
	bgez	v0,.mem_card_open_loop_exit_2
	sw	v0,-16(s0)
	jal	MemCardCallback
	addu	a0,zero,zero
	lw	v1,-32(s0)
	lui	at,((.bss_15)+$8000)>>16
	opt	m-
	blez	v1,.mem_card_open_else_2
	sw	v0,((.bss_15)-((((.bss_15)+$8000)>>16)<<16))(at)
	opt	m+
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_open_skip
	addu	a0,zero,zero
.mem_card_open_else_2:
	la	a0,.mem_card_accept_block
	sw	s1,-32(s0)
	sw	zero,4(s4)
	sw	zero,8(s4)
	lui	at,((.bss_10)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	s3,((.bss_10)-((((.bss_10)+$8000)>>16)<<16))(at)
	opt	m+
	addu	a0,zero,zero
.mem_card_open_skip:
	addu	a1,zero,zero
	jal	MemCardSync
	addiu	a2,sp,16
	lw	a0,.bss_15
	jal	MemCardCallback
	nop
	lw	v1,16(sp)
	nop
	beq	v1,s6,.mem_card_open_loop
	nop
	bne	v1,s1,.mem_card_open_loop_exit
	nop
	addiu	s2,s2,1
	slti	v0,s2,5
	bne	v0,zero,.mem_card_open_loop
	nop
.mem_card_open_loop_exit:
	bne	v1,zero,.mem_card_open_skip_2
	nop
	sw	s7,16(sp)
.mem_card_open_skip_2:
	lw	v0,16(sp)
	j	.mem_card_open_return
	nop
.mem_card_open_loop_exit_2:
	addu	v0,zero,zero
.mem_card_open_return:
	lw	ra,56(sp)
	lw	s7,52(sp)
	lw	s6,48(sp)
	lw	s5,44(sp)
	lw	s4,40(sp)
	lw	s3,36(sp)
	lw	s2,32(sp)
	lw	s1,28(sp)
	lw	s0,24(sp)
	jr	ra
	addiu	sp,sp,64

MemCardClose:
	addiu	sp,sp,-24
	sw	s0,16(sp)
	la	s0,.bss_11
	sw	ra,20(sp)
	lw	a0,0(s0)
	nop
	bltz	a0,.mem_card_close_return
	nop
	jal	close
	nop
	addiu	v0,zero,-1
	sw	v0,0(s0)
.mem_card_close_return:
	lw	ra,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,24

MemCardReadData:
	addiu	sp,sp,-24
	la	a3,.bss_11
	sw	ra,16(sp)
	lw	v0,0(a3)
	nop
	bgez	v0,.mem_card_read_data_else
	addu	t0,a0,zero
	lui	a0,((.rdata_4)+$8000)>>16
	opt	m-
	j	.mem_card_read_data_join
	addiu	a0,a0,((.rdata_4)-((((.rdata_4)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_data_else:
	lw	v0,-16(a3)
	nop
	blez	v0,.mem_card_read_data_else_2
	addiu	v1,a3,-16
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	j	.mem_card_read_data_join
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_data_else_2:
	andi	v0,a2,$7f
	beq	v0,zero,.mem_card_read_data_else_3
	andi	v0,a1,$7f
	lui	a0,((.rdata_5)+$8000)>>16
	opt	m-
	j	.mem_card_read_data_join
	addiu	a0,a0,((.rdata_5)-((((.rdata_5)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_data_else_3:
	bne	v0,zero,.mem_card_read_data_else_4
	addiu	v0,zero,5
	la	a0,.mem_card_read_data_block
	sw	v0,-16(a3)
	sw	zero,4(v1)
	sw	zero,8(v1)
	sw	a1,20(v1)
	sw	t0,28(v1)
	jal	UserFuncOpen
	sw	a2,24(v1)
	j	.mem_card_read_data_return
	addiu	v0,zero,1
.mem_card_read_data_else_4:
	la	a0,.rdata_6
.mem_card_read_data_join:
	jal	printf
	nop
	addu	v0,zero,zero
.mem_card_read_data_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
.mem_card_read_data_block:
	addiu	sp,sp,-32
	sw	s1,20(sp)
	addu	s1,a0,zero
	sw	ra,24(sp)
	sw	s0,16(sp)
	lw	v1,0(s1)
	addiu	s0,zero,10
	beq	v1,s0,.mem_card_read_data_skip
	slti	v0,v1,11
	beq	v0,zero,.mem_card_read_data_else_5
	addiu	v0,zero,30
	beq	v1,zero,.mem_card_read_data_else_6
	addu	v0,zero,zero
	j	.mem_card_read_data_return_3
	nop
.mem_card_read_data_else_5:
	beq	v1,v0,.mem_card_read_data_else_7
	addu	v0,zero,zero
	j	.mem_card_read_data_return_3
	nop
.mem_card_read_data_else_6:
	opt	at+
	sw	zero,.data_1
	opt	at-
	sw	s0,0(s1)
.mem_card_read_data_skip:
	la	s0,.bss_11
.mem_card_read_data_loop:
	lw	a0,0(s0)
	lw	a1,4(s0)
	jal	lseek
	addu	a2,zero,zero
	lw	v1,4(s0)
	nop
	bne	v0,v1,.mem_card_read_data_loop
	nop
	jal	_clr_card_event
	nop
	la	s0,.bss_11
.mem_card_read_data_loop_2:
	lw	a0,0(s0)
	lw	a1,12(s0)
	lw	a2,8(s0)
	jal	read
	nop
	bne	v0,zero,.mem_card_read_data_loop_2
	addiu	v0,zero,30
	j	.mem_card_read_data_return_2
	sw	v0,0(s1)
.mem_card_read_data_else_7:
	jal	_chk_card_event
	nop
	beq	v0,zero,.mem_card_read_data_return_3
	addu	v0,zero,zero
	jal	_get_card_event
	nop
	addu	a0,v0,zero
	beq	a0,zero,.mem_card_read_data_else_8
	nop
	lw	v0,.data_1
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.data_1
	opt	at-
	slti	v0,v0,4
	beq	v0,zero,.mem_card_read_data_skip_2
	nop
	j	.mem_card_read_data_return_2
	sw	s0,0(s1)
.mem_card_read_data_else_8:
	addu	a0,zero,zero
.mem_card_read_data_skip_2:
	jal	.mem_card_unformat_helper
	nop
	la	v1,.bss_7
	sw	v0,4(v1)
	j	.mem_card_read_data_return_3
	addiu	v0,zero,1
.mem_card_read_data_return_2:
	addu	v0,zero,zero
.mem_card_read_data_return_3:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardWriteData:
	addiu	sp,sp,-24
	la	a3,.bss_11
	sw	ra,16(sp)
	lw	v0,0(a3)
	nop
	bgez	v0,.mem_card_write_data_else
	addu	t0,a0,zero
	lui	a0,((.rdata_4)+$8000)>>16
	opt	m-
	j	.mem_card_write_data_join
	addiu	a0,a0,((.rdata_4)-((((.rdata_4)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_data_else:
	lw	v0,-16(a3)
	nop
	blez	v0,.mem_card_write_data_else_2
	addiu	v1,a3,-16
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	j	.mem_card_write_data_join
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_data_else_2:
	andi	v0,a2,$7f
	beq	v0,zero,.mem_card_write_data_else_3
	andi	v0,a1,$7f
	lui	a0,((.rdata_5)+$8000)>>16
	opt	m-
	j	.mem_card_write_data_join
	addiu	a0,a0,((.rdata_5)-((((.rdata_5)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_data_else_3:
	bne	v0,zero,.mem_card_write_data_else_4
	addiu	v0,zero,6
	la	a0,.mem_card_write_data_block
	sw	v0,-16(a3)
	sw	zero,4(v1)
	sw	zero,8(v1)
	sw	a1,20(v1)
	sw	t0,28(v1)
	jal	UserFuncOpen
	sw	a2,24(v1)
	j	.mem_card_write_data_return
	addiu	v0,zero,1
.mem_card_write_data_else_4:
	la	a0,.rdata_6
.mem_card_write_data_join:
	jal	printf
	nop
	addu	v0,zero,zero
.mem_card_write_data_return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
.mem_card_write_data_block:
	addiu	sp,sp,-32
	sw	s1,20(sp)
	addu	s1,a0,zero
	sw	ra,24(sp)
	sw	s0,16(sp)
	lw	v1,0(s1)
	addiu	s0,zero,10
	beq	v1,s0,.mem_card_write_data_else_9
	slti	v0,v1,11
	beq	v0,zero,.mem_card_write_data_else_5
	addiu	v0,zero,30
	beq	v1,zero,.mem_card_write_data_else_6
	addu	v0,zero,zero
	j	.mem_card_write_data_return_3
	nop
.mem_card_write_data_else_5:
	beq	v1,v0,.mem_card_write_data_else_7
	addu	v0,zero,zero
	j	.mem_card_write_data_return_3
	nop
.mem_card_write_data_else_6:
	opt	at+
	sw	zero,.data_2
	opt	at-
	j	.mem_card_write_data_return_2
	sw	s0,0(s1)
.mem_card_write_data_else_7:
	jal	_chk_card_event
	nop
	beq	v0,zero,.mem_card_write_data_return_3
	addu	v0,zero,zero
	jal	_get_card_event
	nop
	addu	a0,v0,zero
	beq	a0,zero,.mem_card_write_data_else_8
	nop
	lw	v0,.data_2
	nop
	addiu	v0,v0,1
	opt	at+
	sw	v0,.data_2
	opt	at-
	slti	v0,v0,4
	beq	v0,zero,.mem_card_write_data_skip
	nop
	j	.mem_card_write_data_return_2
	sw	s0,0(s1)
.mem_card_write_data_else_8:
	addu	a0,zero,zero
.mem_card_write_data_skip:
	jal	.mem_card_unformat_helper
	nop
	la	v1,.bss_7
	sw	v0,4(v1)
	j	.mem_card_write_data_return_3
	addiu	v0,zero,1
.mem_card_write_data_else_9:
	la	s0,.bss_11
.mem_card_write_data_loop:
	lw	a0,0(s0)
	lw	a1,4(s0)
	jal	lseek
	addu	a2,zero,zero
	lw	v1,4(s0)
	nop
	bne	v0,v1,.mem_card_write_data_loop
	nop
	jal	_clr_card_event
	nop
	la	s0,.bss_11
.mem_card_write_data_loop_2:
	lw	a0,0(s0)
	lw	a1,12(s0)
	lw	a2,8(s0)
	jal	write
	nop
	bne	v0,zero,.mem_card_write_data_loop_2
	addiu	v0,zero,30
	sw	v0,0(s1)
.mem_card_write_data_return_2:
	addu	v0,zero,zero
.mem_card_write_data_return_3:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardReadFile:
	addiu	sp,sp,-48
	sw	s4,32(sp)
	addu	s4,a0,zero
	sw	s3,28(sp)
	addu	s3,a1,zero
	sw	s5,36(sp)
	addu	s5,a2,zero
	sw	s2,24(sp)
	addu	s2,a3,zero
	sw	s1,20(sp)
	la	s1,.bss_7
	sw	ra,44(sp)
	sw	s6,40(sp)
	sw	s0,16(sp)
	lw	v0,0(s1)
	lw	s6,64(sp)
	blez	v0,.mem_card_read_file_else
	nop
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	j	.mem_card_read_file_join
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_file_else:
	lw	v0,16(s1)
	nop
	bltz	v0,.mem_card_read_file_else_2
	andi	v0,s6,$7f
	lui	a0,((.rdata_3)+$8000)>>16
	opt	m-
	j	.mem_card_read_file_join
	addiu	a0,a0,((.rdata_3)-((((.rdata_3)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_file_else_2:
	beq	v0,zero,.mem_card_read_file_else_3
	andi	v0,s2,$7f
	lui	a0,((.rdata_5)+$8000)>>16
	opt	m-
	j	.mem_card_read_file_join
	addiu	a0,a0,((.rdata_5)-((((.rdata_5)+$8000)>>16)<<16))
	opt	m+
.mem_card_read_file_else_3:
	bne	v0,zero,.mem_card_read_file_else_4
	addu	a0,s4,zero
	addiu	s0,s1,32
	jal	.mem_card_unformat_helper_2
	addu	a1,s0,zero
	addu	a0,s0,zero
	jal	strcat
	addu	a1,s3,zero
	la	a0,.mem_card_read_file_block
	addiu	v0,zero,3
	sw	v0,0(s1)
	sw	zero,4(s1)
	sw	zero,8(s1)
	sw	s2,20(s1)
	sw	s5,28(s1)
	sw	s6,24(s1)
	jal	UserFuncOpen
	sw	s4,12(s1)
	j	.mem_card_read_file_return
	addiu	v0,zero,1
.mem_card_read_file_else_4:
	la	a0,.rdata_6
.mem_card_read_file_join:
	jal	printf
	nop
	addu	v0,zero,zero
.mem_card_read_file_return:
	lw	ra,44(sp)
	lw	s6,40(sp)
	lw	s5,36(sp)
	lw	s4,32(sp)
	lw	s3,28(sp)
	lw	s2,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,48
.mem_card_read_file_block:
	addiu	sp,sp,-32
	sw	s1,20(sp)
	addu	s1,a0,zero
	sw	ra,24(sp)
	sw	s0,16(sp)
	lw	v1,0(s1)
	addiu	s0,zero,10
	beq	v1,s0,.mem_card_read_file_else_7
	slti	v0,v1,11
	beq	v0,zero,.mem_card_read_file_else_5
	addiu	v0,zero,11
	beq	v1,zero,.mem_card_read_file_else_6
	addu	v0,zero,zero
	j	.mem_card_read_file_return_3
	nop
.mem_card_read_file_else_5:
	beq	v1,v0,.mem_card_read_file_else_8
	addiu	v0,zero,20
	beq	v1,v0,.mem_card_read_file_else_9
	addu	v0,zero,zero
	j	.mem_card_read_file_return_3
	nop
.mem_card_read_file_else_6:
	la	a0,.mem_card_exist_block
	lui	at,((.data_3)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	zero,((.data_3)-((((.data_3)+$8000)>>16)<<16))(at)
	opt	m+
	j	.mem_card_read_file_return_2
	sw	s0,0(s1)
.mem_card_read_file_else_7:
	la	s0,.bss_8
	lw	v0,0(s0)
	nop
	bne	v0,zero,.mem_card_read_file_return_3
	addiu	v0,zero,1
	addiu	a0,s0,28
	jal	open
	ori	a1,zero,$8001
	bgez	v0,.mem_card_read_file_else_8
	sw	v0,12(s0)
	addiu	v1,s0,-4
	addiu	v0,zero,5
	sw	v0,4(v1)
	j	.mem_card_read_file_return_3
	addiu	v0,zero,1
.mem_card_read_file_else_8:
	addiu	v0,zero,20
	la	a0,.mem_card_read_data_block
	jal	UserFuncOpen
	sw	v0,0(s1)
	j	.mem_card_read_file_return_3
	addu	v0,zero,zero
.mem_card_read_file_else_9:
	la	s0,.bss_11
	lw	a0,0(s0)
	jal	close
	nop
	addiu	v0,zero,1
	addiu	v1,zero,-1
	j	.mem_card_read_file_return_3
	sw	v1,0(s0)
.mem_card_read_file_return_2:
	addu	v0,zero,zero
.mem_card_read_file_return_3:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardWriteFile:
	addiu	sp,sp,-48
	sw	s4,32(sp)
	addu	s4,a0,zero
	sw	s3,28(sp)
	addu	s3,a1,zero
	sw	s5,36(sp)
	addu	s5,a2,zero
	sw	s2,24(sp)
	addu	s2,a3,zero
	sw	s1,20(sp)
	la	s1,.bss_7
	sw	ra,44(sp)
	sw	s6,40(sp)
	sw	s0,16(sp)
	lw	v0,0(s1)
	lw	s6,64(sp)
	blez	v0,.mem_card_write_file_else
	nop
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	j	.mem_card_write_file_join
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_file_else:
	lw	v0,16(s1)
	nop
	bltz	v0,.mem_card_write_file_else_2
	andi	v0,s6,$7f
	lui	a0,((.rdata_3)+$8000)>>16
	opt	m-
	j	.mem_card_write_file_join
	addiu	a0,a0,((.rdata_3)-((((.rdata_3)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_file_else_2:
	beq	v0,zero,.mem_card_write_file_else_3
	andi	v0,s2,$7f
	lui	a0,((.rdata_5)+$8000)>>16
	opt	m-
	j	.mem_card_write_file_join
	addiu	a0,a0,((.rdata_5)-((((.rdata_5)+$8000)>>16)<<16))
	opt	m+
.mem_card_write_file_else_3:
	bne	v0,zero,.mem_card_write_file_else_4
	addu	a0,s4,zero
	addiu	s0,s1,32
	jal	.mem_card_unformat_helper_2
	addu	a1,s0,zero
	addu	a0,s0,zero
	jal	strcat
	addu	a1,s3,zero
	la	a0,.mem_card_write_file_block
	addiu	v0,zero,4
	sw	v0,0(s1)
	sw	zero,4(s1)
	sw	zero,8(s1)
	sw	s2,20(s1)
	sw	s5,28(s1)
	sw	s6,24(s1)
	jal	UserFuncOpen
	sw	s4,12(s1)
	j	.mem_card_write_file_return
	addiu	v0,zero,1
.mem_card_write_file_else_4:
	la	a0,.rdata_6
.mem_card_write_file_join:
	jal	printf
	nop
	addu	v0,zero,zero
.mem_card_write_file_return:
	lw	ra,44(sp)
	lw	s6,40(sp)
	lw	s5,36(sp)
	lw	s4,32(sp)
	lw	s3,28(sp)
	lw	s2,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,48
.mem_card_write_file_block:
	addiu	sp,sp,-32
	sw	s1,20(sp)
	addu	s1,a0,zero
	sw	ra,24(sp)
	sw	s0,16(sp)
	lw	v1,0(s1)
	addiu	s0,zero,10
	beq	v1,s0,.mem_card_write_file_else_7
	slti	v0,v1,11
	beq	v0,zero,.mem_card_write_file_else_5
	addiu	v0,zero,11
	beq	v1,zero,.mem_card_write_file_else_6
	addu	v0,zero,zero
	j	.mem_card_write_file_return_3
	nop
.mem_card_write_file_else_5:
	beq	v1,v0,.mem_card_write_file_else_8
	addiu	v0,zero,20
	beq	v1,v0,.mem_card_write_file_else_9
	addu	v0,zero,zero
	j	.mem_card_write_file_return_3
	nop
.mem_card_write_file_else_6:
	la	a0,.mem_card_exist_block
	lui	at,((.data_4)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	zero,((.data_4)-((((.data_4)+$8000)>>16)<<16))(at)
	opt	m+
	j	.mem_card_write_file_return_2
	sw	s0,0(s1)
.mem_card_write_file_else_7:
	la	s0,.bss_8
	lw	v0,0(s0)
	nop
	bne	v0,zero,.mem_card_write_file_return_3
	addiu	v0,zero,1
	addiu	a0,s0,28
	jal	open
	ori	a1,zero,$8001
	bgez	v0,.mem_card_write_file_else_8
	sw	v0,12(s0)
	addiu	v1,s0,-4
	addiu	v0,zero,5
	sw	v0,4(v1)
	j	.mem_card_write_file_return_3
	addiu	v0,zero,1
.mem_card_write_file_else_8:
	addiu	v0,zero,20
	la	a0,.mem_card_write_data_block
	jal	UserFuncOpen
	sw	v0,0(s1)
	j	.mem_card_write_file_return_3
	addu	v0,zero,zero
.mem_card_write_file_else_9:
	la	s0,.bss_11
	lw	a0,0(s0)
	jal	close
	nop
	addiu	v0,zero,1
	addiu	v1,zero,-1
	j	.mem_card_write_file_return_3
	sw	v1,0(s0)
.mem_card_write_file_return_2:
	addu	v0,zero,zero
.mem_card_write_file_return_3:
	lw	ra,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,32

MemCardGetDirentry:
	addiu	sp,sp,-144
	sw	s7,132(sp)
	addu	s7,a0,zero
	sw	s0,104(sp)
	addu	s0,a1,zero
	sw	s6,128(sp)
	addu	s6,a2,zero
	sw	s3,116(sp)
	la	s3,.bss_7
	sw	ra,140(sp)
	sw	fp,136(sp)
	sw	s5,124(sp)
	sw	s4,120(sp)
	sw	s2,112(sp)
	sw	s1,108(sp)
	sw	a3,92(sp)
	lw	v0,0(s3)
	lw	fp,160(sp)
	beq	v0,zero,.mem_card_get_direntry_else
	addu	a0,s7,zero
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_get_direntry_return
	addiu	v0,zero,-1
.mem_card_get_direntry_else:
	jal	.mem_card_unformat_helper_2
	addiu	a1,sp,16
	addiu	a0,sp,16
	jal	strcat
	addu	a1,s0,zero
	addu	s2,zero,zero
	addu	s1,zero,zero
	addiu	v1,zero,1
	sw	zero,88(sp)
	lw	a0,12(s3)
	lw	v0,.bss_6
	lw	t0,164(sp)
	sllv	v1,v1,a0
	or	v0,v0,v1
	opt	at+
	sw	v0,.bss_6
	opt	at-
	addu	v0,fp,t0
	blez	v0,.mem_card_get_direntry_loop_exit_3
	addu	s5,zero,zero
	addiu	t0,sp,80
	sw	t0,96(sp)
	addu	s4,zero,zero
.mem_card_get_direntry_loop:
	bne	s1,zero,.mem_card_get_direntry_else_3
	nop
.mem_card_get_direntry_loop_2:
	jal	_clr_card_event
	nop
	addiu	a0,sp,16
	jal	firstfile
	addiu	a1,sp,48
	addu	s0,v0,zero
	bne	s0,zero,.mem_card_get_direntry_loop_exit_2
	slt	v0,s1,fp
	jal	_get_card_event_x
	nop
	jal	.mem_card_unformat_helper
	addu	a0,v0,zero
	beq	v0,zero,.mem_card_get_direntry_loop_exit
	sw	v0,88(sp)
	addiu	s2,s2,1
	slti	v0,s2,4
	bne	v0,zero,.mem_card_get_direntry_loop_2
	nop
	jal	MemCardCallback
	addu	a0,zero,zero
	lw	v1,0(s3)
	opt	at+
	sw	v0,.bss_15
	opt	at-
	blez	v1,.mem_card_get_direntry_else_2
	addiu	t0,zero,2
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_get_direntry_skip
	addu	a0,zero,zero
.mem_card_get_direntry_else_2:
	la	a0,.mem_card_accept_block
	sw	t0,0(s3)
	sw	zero,4(s3)
	sw	zero,8(s3)
	lui	at,((.bss_10)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	s7,((.bss_10)-((((.bss_10)+$8000)>>16)<<16))(at)
	opt	m+
	addu	a0,zero,zero
.mem_card_get_direntry_skip:
	addu	a1,zero,zero
	jal	MemCardSync
	addiu	a2,sp,88
	lw	a0,.bss_15
	jal	MemCardCallback
	nop
	lw	v0,88(sp)
	j	.mem_card_get_direntry_return
	nop
.mem_card_get_direntry_else_3:
	jal	nextfile
	addiu	a0,sp,48
	addu	s0,v0,zero
.mem_card_get_direntry_loop_exit:
	beq	s0,zero,.mem_card_get_direntry_loop_exit_3
	slt	v0,s1,fp
.mem_card_get_direntry_loop_exit_2:
	bne	v0,zero,.mem_card_get_direntry_next
	nop
	beq	s6,zero,.mem_card_get_direntry_next
	addu	a3,s4,s6
	addiu	a2,sp,48
.mem_card_get_direntry_loop_3:
	lw	v0,0(a2)
	lw	v1,4(a2)
	lw	a0,8(a2)
	lw	a1,12(a2)
	sw	v0,0(a3)
	sw	v1,4(a3)
	sw	a0,8(a3)
	sw	a1,12(a3)
	addiu	a2,a2,16
	lw	t0,96(sp)
	nop
	bne	a2,t0,.mem_card_get_direntry_loop_3
	addiu	a3,a3,16
	lw	v0,0(a2)
	lw	v1,4(a2)
	sw	v0,0(a3)
	sw	v1,4(a3)
	addiu	s4,s4,40
	addiu	s5,s5,1
.mem_card_get_direntry_next:
	lw	t0,164(sp)
	addiu	s1,s1,1
	addu	v0,fp,t0
	slt	v0,s1,v0
	bne	v0,zero,.mem_card_get_direntry_loop
	nop
.mem_card_get_direntry_loop_exit_3:
	lw	t0,92(sp)
	nop
	beq	t0,zero,.mem_card_get_direntry_return
	addu	v0,zero,zero
	sw	s5,0(t0)
.mem_card_get_direntry_return:
	lw	ra,140(sp)
	lw	fp,136(sp)
	lw	s7,132(sp)
	lw	s6,128(sp)
	lw	s5,124(sp)
	lw	s4,120(sp)
	lw	s3,116(sp)
	lw	s2,112(sp)
	lw	s1,108(sp)
	lw	s0,104(sp)
	jr	ra
	addiu	sp,sp,144

MemCardCallback:
	la	v1,.bss_12
	lw	v0,0(v1)
	jr	ra
	sw	a0,0(v1)

MemCardSync:
	la	v1,.bss_7
	lw	v0,0(v1)
	nop
	bne	v0,zero,.mem_card_sync_skip
	nop
	lw	v0,8(v1)
	nop
	beq	v0,zero,.mem_card_sync_return_2
	addiu	v0,zero,-1
.mem_card_sync_skip:
	lw	t0,0(v1)
	lw	a3,4(v1)
	bne	a0,zero,.mem_card_sync_else
	nop
	lw	v0,8(v1)
	nop
	bne	v0,zero,.mem_card_sync_loop_exit
	nop
	addiu	v1,v1,8
.mem_card_sync_loop:
	lw	v0,0(v1)
	nop
	beq	v0,zero,.mem_card_sync_loop
	nop
.mem_card_sync_loop_exit:
	beq	a2,zero,.mem_card_sync_skip_2
	nop
	la	v0,.bss_14
	lw	v0,0(v0)
	nop
	sw	v0,0(a2)
.mem_card_sync_skip_2:
	beq	a1,zero,.mem_card_sync_skip_3
	nop
	la	v0,.bss_13
	lw	v0,0(v0)
	nop
	sw	v0,0(a1)
.mem_card_sync_skip_3:
	la	v0,.bss_7
	sw	zero,8(v0)
	j	.mem_card_sync_return_2
	addiu	v0,zero,1
.mem_card_sync_else:
	lw	v0,8(v1)
	nop
	bne	v0,zero,.mem_card_sync_else_2
	nop
	beq	a2,zero,.mem_card_sync_skip_4
	nop
	sw	a3,0(a2)
.mem_card_sync_skip_4:
	beq	a1,zero,.mem_card_sync_return_2
	addu	v0,zero,zero
	j	.mem_card_sync_return_2
	sw	t0,0(a1)
.mem_card_sync_else_2:
	beq	a2,zero,.mem_card_sync_skip_5
	nop
	la	v0,.bss_14
	lw	v0,0(v0)
	nop
	sw	v0,0(a2)
.mem_card_sync_skip_5:
	beq	a1,zero,.mem_card_sync_return
	nop
	la	v0,.bss_13
	lw	v0,0(v0)
	nop
	sw	v0,0(a1)
.mem_card_sync_return:
	sw	zero,8(v1)
	addiu	v0,zero,1
.mem_card_sync_return_2:
	jr	ra
	nop

MemCardCreateFile:
	addiu	sp,sp,-80
	sw	s3,68(sp)
	addu	s3,a0,zero
	sw	s0,56(sp)
	addu	s0,a1,zero
	sw	s4,72(sp)
	addu	s4,a2,zero
	sw	s2,64(sp)
	la	s2,.bss_7
	sw	ra,76(sp)
	sw	s1,60(sp)
	lw	v0,0(s2)
	nop
	beq	v0,zero,.mem_card_create_file_else
	addu	s1,zero,zero
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_create_file_return
	addiu	v0,zero,-1
.mem_card_create_file_else:
	addu	a0,s3,zero
	jal	.mem_card_unformat_helper_2
	addiu	a1,sp,16
	addiu	a0,sp,16
	jal	strcat
	addu	a1,s0,zero
	addiu	a0,sp,16
	addiu	a1,zero,1
	addiu	v0,zero,1
	lw	a2,12(s2)
	lw	v1,.bss_6
	sllv	v0,v0,a2
	or	v1,v1,v0
	lui	at,((.bss_6)+$8000)>>16
	opt	m-
	jal	open
	sw	v1,((.bss_6)-((((.bss_6)+$8000)>>16)<<16))(at)
	opt	m+
	bltz	v0,.mem_card_create_file_else_2
	sll	s4,s4,16
	jal	close
	addu	a0,v0,zero
	j	.mem_card_create_file_return
	addiu	v0,zero,6
.mem_card_create_file_loop:
	j	.mem_card_create_file_return
	addiu	v0,zero,7
.mem_card_create_file_else_2:
	addu	s0,s2,zero
	addiu	s2,zero,2
.mem_card_create_file_loop_2:
	addiu	a0,sp,16
.mem_card_create_file_loop_3:
	jal	open
	ori	a1,s4,$200
	bgez	v0,.mem_card_create_file_loop_exit_3
	nop
	jal	MemCardCallback
	addu	a0,zero,zero
	lw	v1,0(s0)
	lui	at,((.bss_15)+$8000)>>16
	opt	m-
	blez	v1,.mem_card_create_file_else_3
	sw	v0,((.bss_15)-((((.bss_15)+$8000)>>16)<<16))(at)
	opt	m+
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_create_file_skip
	nop
.mem_card_create_file_else_3:
	la	a0,.mem_card_accept_block
	sw	s2,0(s0)
	sw	zero,4(s0)
	sw	zero,8(s0)
	lui	at,((.bss_10)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	s3,((.bss_10)-((((.bss_10)+$8000)>>16)<<16))(at)
	opt	m+
.mem_card_create_file_skip:
	lw	v0,0(s0)
	nop
	bne	v0,zero,.mem_card_create_file_skip_2
	nop
	lw	v0,8(s0)
	nop
	beq	v0,zero,.mem_card_create_file_skip_3
	nop
.mem_card_create_file_skip_2:
	lw	v0,0(s0)
	lw	v0,4(s0)
	lw	v0,8(s0)
	nop
	bne	v0,zero,.mem_card_create_file_loop_exit
	nop
	la	v1,.bss_9
.mem_card_create_file_loop_4:
	lw	v0,0(v1)
	nop
	beq	v0,zero,.mem_card_create_file_loop_4
	nop
.mem_card_create_file_loop_exit:
	la	v0,.bss_14
	lw	v0,0(v0)
	sw	zero,8(s0)
	sw	v0,48(sp)
.mem_card_create_file_skip_3:
	lw	a0,.bss_15
	jal	MemCardCallback
	nop
	lw	v1,48(sp)
	nop
	beq	v1,zero,.mem_card_create_file_loop
	addiu	v0,zero,3
	beq	v1,v0,.mem_card_create_file_loop_3
	addiu	a0,sp,16
	bne	v1,s2,.mem_card_create_file_loop_exit_2
	nop
	addiu	s1,s1,1
	slti	v0,s1,4
	bne	v0,zero,.mem_card_create_file_loop_2
	nop
.mem_card_create_file_loop_exit_2:
	bne	v1,zero,.mem_card_create_file_skip_4
	addiu	v0,zero,5
	sw	v0,48(sp)
.mem_card_create_file_skip_4:
	lw	v0,48(sp)
	j	.mem_card_create_file_return
	nop
.mem_card_create_file_loop_exit_3:
	jal	close
	addu	a0,v0,zero
	addu	v0,zero,zero
.mem_card_create_file_return:
	lw	ra,76(sp)
	lw	s4,72(sp)
	lw	s3,68(sp)
	lw	s2,64(sp)
	lw	s1,60(sp)
	lw	s0,56(sp)
	jr	ra
	addiu	sp,sp,80

MemCardDeleteFile:
	addiu	sp,sp,-80
	sw	s2,64(sp)
	addu	s2,a0,zero
	sw	s0,56(sp)
	addu	s0,a1,zero
	sw	s3,68(sp)
	la	s3,.bss_7
	sw	ra,72(sp)
	sw	s1,60(sp)
	lw	v0,0(s3)
	nop
	beq	v0,zero,.mem_card_delete_file_else
	addu	s1,zero,zero
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_delete_file_return
	addiu	v0,zero,-1
.mem_card_delete_file_else:
	addu	a0,s2,zero
	jal	.mem_card_unformat_helper_2
	addiu	a1,sp,16
	addiu	a0,sp,16
	jal	strcat
	addu	a1,s0,zero
	addu	s0,s3,zero
	addiu	s3,zero,2
	addiu	v0,zero,1
	lw	a0,12(s0)
	lw	v1,.bss_6
	sllv	v0,v0,a0
	or	v1,v1,v0
	opt	at+
	sw	v1,.bss_6
	opt	at-
.mem_card_delete_file_loop:
	jal	erase
	addiu	a0,sp,16
	bne	v0,zero,.mem_card_delete_file_loop_exit_3
	sw	v0,48(sp)
	jal	MemCardCallback
	addu	a0,zero,zero
	lw	v1,0(s0)
	lui	at,((.bss_15)+$8000)>>16
	opt	m-
	blez	v1,.mem_card_delete_file_else_2
	sw	v0,((.bss_15)-((((.bss_15)+$8000)>>16)<<16))(at)
	opt	m+
	lui	a0,((.rdata_1)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_delete_file_skip
	nop
.mem_card_delete_file_else_2:
	la	a0,.mem_card_accept_block
	sw	s3,0(s0)
	sw	zero,4(s0)
	sw	zero,8(s0)
	lui	at,((.bss_10)+$8000)>>16
	opt	m-
	jal	UserFuncOpen
	sw	s2,((.bss_10)-((((.bss_10)+$8000)>>16)<<16))(at)
	opt	m+
.mem_card_delete_file_skip:
	lw	v0,0(s0)
	nop
	bne	v0,zero,.mem_card_delete_file_skip_2
	nop
	lw	v0,8(s0)
	nop
	beq	v0,zero,.mem_card_delete_file_skip_3
	nop
.mem_card_delete_file_skip_2:
	lw	v0,0(s0)
	lw	v0,4(s0)
	lw	v0,8(s0)
	nop
	bne	v0,zero,.mem_card_delete_file_loop_exit
	nop
	la	v1,.bss_9
.mem_card_delete_file_loop_2:
	lw	v0,0(v1)
	nop
	beq	v0,zero,.mem_card_delete_file_loop_2
	nop
.mem_card_delete_file_loop_exit:
	la	v0,.bss_14
	lw	v0,0(v0)
	sw	zero,8(s0)
	sw	v0,48(sp)
.mem_card_delete_file_skip_3:
	lw	a0,.bss_15
	jal	MemCardCallback
	nop
	lw	v1,48(sp)
	addiu	v0,zero,3
	beq	v1,v0,.mem_card_delete_file_loop
	nop
	bne	v1,s3,.mem_card_delete_file_loop_exit_2
	nop
	addiu	s1,s1,1
	slti	v0,s1,4
	bne	v0,zero,.mem_card_delete_file_loop
	nop
.mem_card_delete_file_loop_exit_2:
	bne	v1,zero,.mem_card_delete_file_skip_4
	addiu	v0,zero,5
	sw	v0,48(sp)
.mem_card_delete_file_skip_4:
	lw	v0,48(sp)
	j	.mem_card_delete_file_return
	nop
.mem_card_delete_file_loop_exit_3:
	addu	v0,zero,zero
.mem_card_delete_file_return:
	lw	ra,72(sp)
	lw	s3,68(sp)
	lw	s2,64(sp)
	lw	s1,60(sp)
	lw	s0,56(sp)
	jr	ra
	addiu	sp,sp,80

MemCardFormat:
	addiu	sp,sp,-88
	la	v1,.bss_7
	sw	ra,80(sp)
	lw	v0,0(v1)
	nop
	bne	v0,zero,.mem_card_format_else
	addiu	v0,zero,1
	lw	a1,12(v1)
	lw	v1,.bss_6
	sllv	v0,v0,a1
	or	v1,v1,v0
	addiu	a1,sp,16
	lui	at,((.bss_6)+$8000)>>16
	opt	m-
	jal	.mem_card_unformat_helper_2
	sw	v1,((.bss_6)-((((.bss_6)+$8000)>>16)<<16))(at)
	opt	m+
	jal	_clr_card_event
	nop
	jal	format
	addiu	a0,sp,16
	jal	_get_card_event_x
	nop
	jal	.mem_card_unformat_helper
	addu	a0,v0,zero
	j	.mem_card_format_return
	nop
.mem_card_format_else:
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
	addiu	v0,zero,-1
.mem_card_format_return:
	lw	ra,80(sp)
	addiu	sp,sp,88
	jr	ra
	nop

MemCardUnformat:
	addiu	sp,sp,-160
	la	v0,.bss_7
	sw	ra,152(sp)
	sw	s1,148(sp)
	sw	s0,144(sp)
	lw	v0,0(v0)
	nop
	beq	v0,zero,.mem_card_unformat_else
	addu	s1,a0,zero
	lui	a0,((.rdata_7)+$8000)>>16
	opt	m-
	jal	printf
	addiu	a0,a0,((.rdata_7)-((((.rdata_7)+$8000)>>16)<<16))
	opt	m+
	j	.mem_card_unformat_return
	addiu	v0,zero,-1
.mem_card_unformat_loop:
	j	.mem_card_unformat_return
	addu	v0,zero,zero
.mem_card_unformat_else:
	addiu	v1,zero,-1
	addiu	s0,zero,127
	addiu	v0,sp,143
.mem_card_unformat_loop_2:
	sb	v1,0(v0)
	addiu	s0,s0,-1
	bgez	s0,.mem_card_unformat_loop_2
	addiu	v0,v0,-1
	addu	s0,zero,zero
.mem_card_unformat_loop_3:
	jal	_clr_card_event
	nop
	jal	_new_card
	nop
	addu	a0,s1,zero
	addu	a1,s0,zero
	jal	_card_write
	addiu	a2,sp,16
	jal	_get_card_event_x
	nop
	bne	v0,zero,.mem_card_unformat_loop
	addiu	s0,s0,1
	slti	v0,s0,15
	bne	v0,zero,.mem_card_unformat_loop_3
	addiu	v0,zero,1
.mem_card_unformat_return:
	lw	ra,152(sp)
	lw	s1,148(sp)
	lw	s0,144(sp)
	jr	ra
	addiu	sp,sp,160
.mem_card_unformat_helper:
	addiu	v0,zero,1
	beq	a0,v0,.mem_card_unformat_return_2
	addu	v1,zero,zero
	slti	v0,a0,2
	beq	v0,zero,.mem_card_unformat_else_2
	nop
	beq	a0,zero,.mem_card_unformat_return_3
	nop
	j	.mem_card_unformat_return_3
	ori	v1,a0,$8000
.mem_card_unformat_else_2:
	addiu	v0,zero,2
	beq	a0,v0,.mem_card_unformat_return_3
	addiu	v1,zero,1
	addiu	v0,zero,4
	bne	a0,v0,.mem_card_unformat_return_3
	ori	v1,a0,$8000
	j	.mem_card_unformat_return_3
	addiu	v1,zero,3
.mem_card_unformat_return_2:
	addiu	v1,zero,2
.mem_card_unformat_return_3:
	jr	ra
	addu	v0,v1,zero
.mem_card_unformat_block:
	addiu	sp,sp,-24
	sw	ra,16(sp)
	jal	UserFuncComplete
	nop
	bne	v0,zero,.mem_card_unformat_return_4
	nop
	jal	UserFuncExecute
	nop
	jal	UserFuncComplete
	nop
	beq	v0,zero,.mem_card_unformat_return_4
	addiu	v0,zero,1
	la	v1,.bss_7
	sw	v0,8(v1)
	lw	v0,0(v1)
	la	a1,.bss_13
	sw	v0,0(a1)
	lw	v0,4(v1)
	lw	a2,64(v1)
	sw	v0,4(a1)
	sw	zero,0(v1)
	beq	a2,zero,.mem_card_unformat_return_4
	sw	zero,4(v1)
	lw	a0,0(a1)
	lw	a1,4(a1)
	jalr	ra,a2
	nop
.mem_card_unformat_return_4:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
.mem_card_unformat_helper_2:
	addu	a2,a0,zero
	addu	a3,a1,zero
	la	a1,.rdata_8
	lwl	v0,3(a1)
	lwr	v0,0(a1)
	lb	v1,4(a1)
	lb	a0,5(a1)
	swl	v0,3(a3)
	swr	v0,0(a3)
	sb	v1,4(a3)
	sb	a0,5(a3)
	bgez	a2,.mem_card_unformat_return_5
	addu	v0,a2,zero
	addiu	v0,a2,15
.mem_card_unformat_return_5:
	sra	v0,v0,4
	addiu	v1,v0,48
	sll	v0,v0,4
	subu	v0,a2,v0
	addiu	v0,v0,48
	sb	v1,2(a3)
	jr	ra
	sb	v0,3(a3)

	section	.rdata
.rdata_1:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$65766520
	dw	$6d20746e
	dw	$69746c75
	dw	$20656c70
	dw	$6e65706f
	dw	$0000000a
.rdata_2:
	dw	.mem_card_accept_case_0
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_10
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_21
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_default
	dw	.mem_card_accept_case_30
	dw	.mem_card_accept_case_31
.rdata_3:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$6c696620
	dw	$6c612065
	dw	$64616572
	dw	$706f2079
	dw	$0a2e6e65
	dw	$00000000
.rdata_4:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$6c696620
	dw	$6f6e2065
	dw	$706f2074
	dw	$0a2e6e65
	dw	$00000000
.rdata_5:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$766e6920
	dw	$64696c61
	dw	$74616420
	dw	$69732061
	dw	$6120657a
	dw	$6e67696c
	dw	$0000000a
.rdata_6:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$766e6920
	dw	$64696c61
	dw	$66666f20
	dw	$20746573
	dw	$756c6176
	dw	$6c612065
	dw	$0a6e6769
	dw	$00000000
.rdata_7:
	dw	$65636341
	dw	$44207373
	dw	$65696e65
	dw	$3a202e64
	dw	$73797320
	dw	$206d6574
	dw	$79737562
	dw	$0000000a
.rdata_8:
	dw	$30307562
	dw	$0000003a
	dw	$00000000
	dw	$00000000

	section	.data
	dw	$25187350
	dw	$0043539b
.data_1:
	dw	$00000000
.data_2:
	dw	$00000000
.data_3:
	dw	$00000000
.data_4:
	dw	$00000000
	dw	$00000000
	dw	$00000000

bss	group	bss
	section	.bss,bss
.bss_1:
	dsb	4
.bss_2:
	dsb	4
.bss_3:
	dsb	4
.bss_4:
	dsb	4
.bss_5:
	dsb	4
.bss_6:
	dsb	4
.bss_7:
	dsb	4
.bss_8:
	dsb	4
.bss_9:
	dsb	4
.bss_10:
	dsb	4
.bss_11:
	dsb	48
.bss_12:
	dsb	8
.bss_13:
	dsb	4
.bss_14:
	dsb	8
.bss_15:
	dsb	4
