; LIBC2 RAND
	opt	c+,at-,m+

	xdef	rand
	xdef	srand

	section	.text

rand:
	lui	v1,$41c6
	lw	v0,.bss_1
	ori	v1,v1,$4e6d
	mult	v0,v1
	mflo	a0
	addiu	v0,a0,12345
	opt	at+
	sw	v0,.bss_1
	opt	at-
	srl	v0,v0,16
	jr	ra
	andi	v0,v0,$7fff

srand:
	lui	at,((.bss_1)+$8000)>>16
	opt	m-
	jr	ra
	sw	a0,((.bss_1)-((((.bss_1)+$8000)>>16)<<16))(at)
	opt	m+
	nop

bss	group	bss
	section	.bss,bss
.bss_1:
	dsb	16
