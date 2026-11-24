; LIBSND VM_NOFF
	opt	c+,at-,m-

	xref	_svm_voice
	xdef	vmNoiseOff

	section	.text

vmNoiseOff:
	andi	a0,a0,$ff
	sll	v0,a0,3
	subu	v0,v0,a0
	sll	v0,v0,2
	subu	v0,v0,a0
	sll	v0,v0,1
	lui	at,((_svm_voice+$1d)+$8000)>>16
	addu	at,at,v0
	sb	zero,((_svm_voice+$1d)-((((_svm_voice+$1d)+$8000)>>16)<<16))(at)
	lui	at,((_svm_voice)+$8000)>>16
	addu	at,at,v0
	sh	zero,((_svm_voice)-((((_svm_voice)+$8000)>>16)<<16))(at)
	lui	at,((_svm_voice+$4)+$8000)>>16
	addu	at,at,v0
	jr	ra
	sh	zero,((_svm_voice+$4)-((((_svm_voice+$4)+$8000)>>16)<<16))(at)
