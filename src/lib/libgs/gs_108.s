; LIBGS GS_108
	opt	c+,at-,m-

	xref	GsLIGHT_MODE
	xref	printf
	xdef	GsSetLightMode

	section	.text

GsSetLightMode:
	addiu	sp,sp,-24
	addu	a1,a0,zero
	addiu	v0,zero,1
	beq	a1,v0,.else_3
	sw	ra,16(sp)
	slti	v0,a1,2
	beq	v0,zero,.else
	addiu	v0,zero,2
	beq	a1,zero,.else_2
	nop
	j	.else_4
	nop
.else:
	beq	a1,v0,.else_3
	addiu	v0,zero,3
	beq	a1,v0,.else_3
	nop
	j	.else_4
	nop
.else_2:
	lui	at,((GsLIGHT_MODE)+$8000)>>16
	j	.return
	sw	zero,((GsLIGHT_MODE)-((((GsLIGHT_MODE)+$8000)>>16)<<16))(at)
.else_3:
	lui	at,((GsLIGHT_MODE)+$8000)>>16
	j	.return
	sw	a1,((GsLIGHT_MODE)-((((GsLIGHT_MODE)+$8000)>>16)<<16))(at)
.else_4:
	lui	a0,((.rdata_1)+$8000)>>16
	jal	printf
	addiu	a0,a0,((.rdata_1)-((((.rdata_1)+$8000)>>16)<<16))
.return:
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
	nop

	section	.rdata
.rdata_1:
	dw	$20746f6e
	dw	$70707573
	dw	$6574726f
	dw	$696c2064
	dw	$20746867
	dw	$65646f6d
	dw	$0a642520
	dw	$00000000
