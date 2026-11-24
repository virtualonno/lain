; LIBSPU S_SIC
	opt	c+,at-,m+

	xref	_SpuCallback
	xref	_spu_IRQCallback
	xdef	SpuSetIRQCallback

	section	.text

SpuSetIRQCallback:
	addiu	sp,sp,-24
	sw	s0,16(sp)
	lw	s0,_spu_IRQCallback
	nop
	beq	a0,s0,.return
	sw	ra,20(sp)
	lui	at,((_spu_IRQCallback)+$8000)>>16
	opt	m-
	jal	_SpuCallback
	sw	a0,((_spu_IRQCallback)-((((_spu_IRQCallback)+$8000)>>16)<<16))(at)
	opt	m+
.return:
	addu	v0,s0,zero
	lw	ra,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,24
	nop
