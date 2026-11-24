; LIBGS GS_010
	opt	c+,at-,m+

	xref	GsDISPENV
	xref	GsSetDrawBuffClip
	xref	GsSetDrawBuffOffset
	xref	PSDCNT
	xref	PSDIDX
	xref	PSDOFSX
	xref	PSDOFSY
	xref	PutDispEnv
	xref	SetDispMask
	xdef	GsSwapDispBuff

	section	.text

GsSwapDispBuff:
	lh	v0,PSDIDX
	addiu	sp,sp,-24
	sw	ra,16(sp)
	sll	v0,v0,1
	lui	v1,((PSDOFSX)+$8000)>>16
	addu	v1,v1,v0
	opt	m-
	lhu	v1,((PSDOFSX)-((((PSDOFSX)+$8000)>>16)<<16))(v1)
	opt	m+
	la	a0,GsDISPENV
	sh	v1,0(a0)
	lui	at,((PSDOFSY)+$8000)>>16
	addu	at,at,v0
	opt	m-
	lhu	v0,((PSDOFSY)-((((PSDOFSY)+$8000)>>16)<<16))(at)
	opt	m+
	jal	PutDispEnv
	sh	v0,2(a0)
	jal	SetDispMask
	addiu	a0,zero,1
	lw	v0,PSDCNT
	nop
	addiu	v0,v0,1
	addu	v1,v0,zero
	lui	at,((PSDCNT)+$8000)>>16
	opt	m-
	bne	v1,zero,.skip
	sw	v1,((PSDCNT)-((((PSDCNT)+$8000)>>16)<<16))(at)
	opt	m+
	addiu	v1,zero,1
.skip:
	lh	v0,PSDIDX
	opt	at+
	sw	v1,PSDCNT
	opt	at-
	sltiu	v0,v0,1
	lui	at,((PSDIDX)+$8000)>>16
	opt	m-
	jal	GsSetDrawBuffClip
	sh	v0,((PSDIDX)-((((PSDIDX)+$8000)>>16)<<16))(at)
	opt	m+
	jal	GsSetDrawBuffOffset
	nop
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop
	nop
	nop
	nop
