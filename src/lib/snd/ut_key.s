; LIBSND UT_KEY
	opt	c+,at-,m+

	xref	_SsVmAlloc
	xref	_SsVmDoAllocate
	xref	_SsVmKeyOffNow
	xref	_SsVmKeyOnNow
	xref	_SsVmMaxVoice
	xref	_SsVmVSetUp
	xref	_snd_ev_flag
	xref	_svm_cur
	xref	_svm_pg
	xref	_svm_tn
	xref	_svm_voice
	xref	note2pitch2
	xref	vmNoiseOff
	xref	vmNoiseOn
	xdef	SsUtKeyOn
	xdef	SsUtKeyOff

	section	.text

SsUtKeyOn:
	addiu	sp,sp,-56
	sw	s0,16(sp)
	lw	s0,76(sp)
	sw	s1,20(sp)
	lw	s1,80(sp)
	lw	v0,_snd_ev_flag
	sw	fp,48(sp)
	addu	fp,a0,zero
	sw	s6,40(sp)
	addu	s6,a1,zero
	sw	s3,28(sp)
	addu	s3,a2,zero
	sw	s5,36(sp)
	addu	s5,a3,zero
	sw	s7,44(sp)
	lhu	s7,72(sp)
	addiu	v1,zero,1
	sw	ra,52(sp)
	sw	s4,32(sp)
	sw	s2,24(sp)
	addu	s2,s0,zero
	bne	v0,v1,.ss_ut_key_on_else
	addu	s4,s1,zero
	j	.ss_ut_key_on_return_2
	addiu	v0,zero,-1
.ss_ut_key_on_else:
	opt	at+
	sw	v1,_snd_ev_flag
	opt	at-
	sll	a0,a0,16
	sra	a0,a0,16
	sll	a1,a1,16
	jal	_SsVmVSetUp
	sra	a1,a1,16
	bne	v0,zero,.ss_ut_key_on_join_3
	addiu	v0,zero,33
	la	a1,_svm_cur+$14
	sh	v0,0(a1)
	sll	v0,s0,16
	sra	v1,v0,16
	sll	v0,s1,16
	sra	a0,v0,16
	sb	s5,-18(a1)
	sb	s7,-17(a1)
	bne	v1,a0,.ss_ut_key_on_else_2
	sb	s3,-8(a1)
	addiu	v0,zero,64
	sb	v0,-15(a1)
	j	.ss_ut_key_on_skip_4
	sb	s2,-16(a1)
.ss_ut_key_on_else_2:
	slt	v0,a0,v1
	beq	v0,zero,.ss_ut_key_on_else_3
	sll	v0,a0,6
	div	v0,v1
	bne	v1,zero,.ss_ut_key_on_skip
	nop
	break	$1c00
.ss_ut_key_on_skip:
	addiu	at,zero,-1
	bne	v1,at,.ss_ut_key_on_join
	lui	at,$8000
	bne	v0,at,.ss_ut_key_on_join
	nop
	break	$1800
.ss_ut_key_on_join:
	mflo	v0
	j	.ss_ut_key_on_skip_3
	sb	s2,-16(a1)
.ss_ut_key_on_else_3:
	sll	v1,v1,6
	div	v1,a0
	bne	a0,zero,.ss_ut_key_on_skip_2
	nop
	break	$1c00
.ss_ut_key_on_skip_2:
	addiu	at,zero,-1
	bne	a0,at,.ss_ut_key_on_join_2
	lui	at,$8000
	bne	v1,at,.ss_ut_key_on_join_2
	nop
	break	$1800
.ss_ut_key_on_join_2:
	mflo	v1
	addiu	v0,zero,127
	sb	s4,-16(a1)
	subu	v0,v0,v1
.ss_ut_key_on_skip_3:
	sb	v0,-15(a1)
.ss_ut_key_on_skip_4:
	sll	v1,s6,16
	lw	v0,_svm_pg
	sra	v1,v1,12
	addu	v1,v1,v0
	lbu	v0,1(v1)
	la	s0,_svm_cur+$a
	sb	v0,0(s0)
	lbu	v0,2(s0)
	lbu	a0,4(v1)
	sll	v0,v0,24
	sb	a0,1(s0)
	lbu	a0,0(v1)
	lb	v1,-3(s0)
	sra	v0,v0,24
	sll	v1,v1,4
	addu	v0,v0,v1
	sll	v0,v0,16
	sb	a0,-10(s0)
	lw	v1,_svm_tn
	sra	v0,v0,11
	addu	v0,v0,v1
	lbu	v1,0(v0)
	nop
	sb	v1,5(s0)
	lhu	a0,22(v0)
	nop
	sh	a0,12(s0)
	lbu	v1,2(v0)
	nop
	sb	v1,3(s0)
	lbu	v1,3(v0)
	nop
	sb	v1,4(s0)
	lbu	v1,4(v0)
	nop
	sb	v1,6(s0)
	lbu	v1,5(v0)
	sll	a0,a0,16
	sb	v1,7(s0)
	lbu	v0,1(v0)
	sra	a0,a0,16
	beq	a0,zero,.ss_ut_key_on_join_3
	sb	v0,8(s0)
	jal	_SsVmAlloc
	nop
	andi	s1,v0,$ff
	lb	v0,_SsVmMaxVoice
	addu	v1,s1,zero
	bne	v1,v0,.ss_ut_key_on_else_4
	sll	v0,v1,3
.ss_ut_key_on_join_3:
	opt	at+
	sw	zero,_snd_ev_flag
	opt	at-
	j	.ss_ut_key_on_return_2
	addiu	v0,zero,-1
.ss_ut_key_on_else_4:
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	addiu	v1,zero,33
	sh	s1,14(s0)
	lui	at,((_svm_voice+$10)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	v1,((_svm_voice+$10)-((((_svm_voice+$10)+$8000)>>16)<<16))(at)
	opt	m+
	lui	at,((_svm_voice+$18)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	fp,((_svm_voice+$18)-((((_svm_voice+$18)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	v1,-3(s0)
	lui	at,((_svm_voice+$14)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	s6,((_svm_voice+$14)-((((_svm_voice+$14)+$8000)>>16)<<16))(at)
	opt	m+
	sll	v1,v1,24
	sra	v1,v1,24
	lui	at,((_svm_voice+$12)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	v1,((_svm_voice+$12)-((((_svm_voice+$12)+$8000)>>16)<<16))(at)
	opt	m+
	lhu	v1,12(s0)
	lui	at,((_svm_voice)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	v1,((_svm_voice)-((((_svm_voice)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	a0,2(s0)
	addiu	v1,zero,1
	lui	at,((_svm_voice+$e)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	s5,((_svm_voice+$e)-((((_svm_voice+$e)+$8000)>>16)<<16))(at)
	opt	m+
	lui	at,((_svm_voice+$1d)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sb	v1,((_svm_voice+$1d)-((((_svm_voice+$1d)+$8000)>>16)<<16))(at)
	opt	m+
	lui	at,((_svm_voice+$2)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	zero,((_svm_voice+$2)-((((_svm_voice+$2)+$8000)>>16)<<16))(at)
	opt	m+
	sll	a0,a0,24
	sra	a0,a0,24
	lui	at,((_svm_voice+$16)+$8000)>>16
	addu	at,at,v0
	opt	m-
	jal	_SsVmDoAllocate
	sh	a0,((_svm_voice+$16)-((((_svm_voice+$16)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,12(s0)
	addiu	v0,zero,255
	bne	v1,v0,.ss_ut_key_on_else_5
	andi	a0,s5,$ffff
	jal	vmNoiseOn
	addu	a0,s1,zero
	j	.ss_ut_key_on_return
	nop
.ss_ut_key_on_else_5:
	jal	note2pitch2
	addu	a1,s7,zero
	addiu	a0,zero,1
	jal	_SsVmKeyOnNow
	andi	a1,v0,$ffff
.ss_ut_key_on_return:
	opt	at+
	sw	zero,_snd_ev_flag
	opt	at-
	addu	v0,s1,zero
.ss_ut_key_on_return_2:
	lw	ra,52(sp)
	lw	fp,48(sp)
	lw	s7,44(sp)
	lw	s6,40(sp)
	lw	s5,36(sp)
	lw	s4,32(sp)
	lw	s3,28(sp)
	lw	s2,24(sp)
	lw	s1,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,56

SsUtKeyOff:
	addiu	sp,sp,-24
	sw	s0,16(sp)
	addu	s0,a0,zero
	lw	v0,_snd_ev_flag
	lw	t0,40(sp)
	addiu	v1,zero,1
	beq	v0,v1,.ss_ut_key_off_return_2
	sw	ra,20(sp)
	opt	at+
	sw	v1,_snd_ev_flag
	opt	at-
	andi	v0,s0,$ffff
	sltiu	v0,v0,24
	beq	v0,zero,.ss_ut_key_off_return
	sll	v1,a0,16
	sra	v1,v1,16
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	a0,v0,1
	lui	v1,((_svm_voice+$18)+$8000)>>16
	addu	v1,v1,a0
	opt	m-
	lh	v1,((_svm_voice+$18)-((((_svm_voice+$18)+$8000)>>16)<<16))(v1)
	opt	m+
	sll	v0,a1,16
	sra	v0,v0,16
	bne	v1,v0,.ss_ut_key_off_else
	sll	v0,a2,16
	lui	v1,((_svm_voice+$14)+$8000)>>16
	addu	v1,v1,a0
	opt	m-
	lh	v1,((_svm_voice+$14)-((((_svm_voice+$14)+$8000)>>16)<<16))(v1)
	opt	m+
	sra	v0,v0,16
	bne	v1,v0,.ss_ut_key_off_else
	sll	v0,a3,16
	lui	v1,((_svm_voice+$16)+$8000)>>16
	addu	v1,v1,a0
	opt	m-
	lh	v1,((_svm_voice+$16)-((((_svm_voice+$16)+$8000)>>16)<<16))(v1)
	opt	m+
	sra	v0,v0,16
	bne	v1,v0,.ss_ut_key_off_else
	sll	v0,t0,16
	lui	v1,((_svm_voice+$e)+$8000)>>16
	addu	v1,v1,a0
	opt	m-
	lh	v1,((_svm_voice+$e)-((((_svm_voice+$e)+$8000)>>16)<<16))(v1)
	opt	m+
	sra	v0,v0,16
	bne	v1,v0,.ss_ut_key_off_else
	addiu	v0,zero,255
	lui	v1,((_svm_voice)+$8000)>>16
	addu	v1,v1,a0
	opt	m-
	lh	v1,((_svm_voice)-((((_svm_voice)+$8000)>>16)<<16))(v1)
	opt	m+
	nop
	bne	v1,v0,.ss_ut_key_off_else
	nop
	jal	vmNoiseOff
	andi	a0,s0,$ff
	j	.ss_ut_key_off_skip
	addu	v0,zero,zero
.ss_ut_key_off_else:
	opt	at+
	sh	s0,_svm_cur+$18
	opt	at-
	jal	_SsVmKeyOffNow
	addu	a0,zero,zero
	addu	v0,zero,zero
.ss_ut_key_off_skip:
	sll	a0,s0,16
	sra	a0,a0,16
	sll	v1,a0,3
	subu	v1,v1,a0
	sll	v1,v1,2
	subu	v1,v1,a0
	sll	v1,v1,1
	lui	at,((_svm_voice+$2a)+$8000)>>16
	addu	at,at,v1
	opt	m-
	sh	zero,((_svm_voice+$2a)-((((_svm_voice+$2a)+$8000)>>16)<<16))(at)
	opt	m+
	lui	at,((_svm_voice+$1e)+$8000)>>16
	addu	at,at,v1
	opt	m-
	sh	zero,((_svm_voice+$1e)-((((_svm_voice+$1e)+$8000)>>16)<<16))(at)
	opt	m+
	lui	at,((_snd_ev_flag)+$8000)>>16
	opt	m-
	j	.ss_ut_key_off_return_3
	sw	zero,((_snd_ev_flag)-((((_snd_ev_flag)+$8000)>>16)<<16))(at)
	opt	m+
.ss_ut_key_off_return:
	opt	at+
	sw	zero,_snd_ev_flag
	opt	at-
.ss_ut_key_off_return_2:
	addiu	v0,zero,-1
.ss_ut_key_off_return_3:
	lw	ra,20(sp)
	lw	s0,16(sp)
	jr	ra
	addiu	sp,sp,24
	nop
	nop
