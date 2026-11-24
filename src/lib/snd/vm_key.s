; LIBSND VM_KEY
	opt	c+,at-,m+

	xref	_SsVmAlloc
	xref	_SsVmDoAllocate
	xref	_SsVmKeyOffNow
	xref	_SsVmKeyOnNow
	xref	_SsVmMaxVoice
	xref	_SsVmSelectToneAndVag
	xref	_SsVmVSetUp
	xref	_snd_vmask
	xref	_ss_score
	xref	_svm_cur
	xref	_svm_pg
	xref	_svm_tn
	xref	_svm_vh
	xref	_svm_voice
	xref	note2pitch
	xref	vmNoiseOff
	xref	vmNoiseOn
	xdef	_SsVmKeyOn
	xdef	_SsVmKeyOff
	xdef	_SsVmSeKeyOn
	xdef	_SsVmSeKeyOff
	xdef	KeyOnCheck

	section	.text

_SsVmKeyOn:
	addiu	sp,sp,-328
	sw	s0,288(sp)
	addu	s0,a0,zero
	addu	v0,a1,zero
	sw	s2,296(sp)
	addu	s2,zero,zero
	sll	a0,v0,16
	sra	a0,a0,16
	sll	a1,a2,16
	sra	a1,a1,16
	sw	s6,312(sp)
	addu	s6,s0,zero
	sw	s3,300(sp)
	addu	s3,v0,zero
	sw	s7,316(sp)
	addu	s7,a3,zero
	sh	a2,272(sp)
	andi	a2,s0,$ff
	sll	a2,a2,2
	sll	v0,s0,16
	sw	s1,292(sp)
	sra	s1,v0,16
	andi	v1,s1,$ff00
	sra	v1,v1,8
	sll	v0,v1,1
	addu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,4
	sw	ra,324(sp)
	sw	fp,320(sp)
	sw	s5,308(sp)
	sw	s4,304(sp)
	lui	v1,((_ss_score)+$8000)>>16
	addu	v1,v1,a2
	opt	m-
	lw	v1,((_ss_score)-((((_ss_score)+$8000)>>16)<<16))(v1)
	opt	m+
	lhu	s4,344(sp)
	lhu	fp,348(sp)
	jal	_SsVmVSetUp
	addu	s5,v1,v0
	bne	v0,zero,.ss_vm_key_on_return_2
	addiu	v0,zero,-1
	la	a0,_svm_cur+$14
	addiu	v0,zero,33
	sh	s0,0(a0)
	sb	s7,-18(a0)
	bne	s1,v0,.ss_vm_key_on_else
	sb	zero,-17(a0)
	j	.ss_vm_key_on_skip
	sb	s4,-16(a0)
.ss_vm_key_on_else:
	lbu	v0,23(s5)
	nop
	sll	v0,v0,1
	addu	v0,v0,s5
	lh	v0,96(v0)
	nop
	mult	s4,v0
	mflo	v0
	li	v1,$81020409
	mult	v0,v1
	mfhi	t0
	addu	v1,t0,v0
	sra	v1,v1,6
	sra	v0,v0,31
	subu	v1,v1,v0
	sb	v1,-16(a0)
.ss_vm_key_on_skip:
	la	s0,_svm_cur+$5
	sb	fp,0(s0)
	lhu	t0,272(sp)
	lw	v1,_svm_pg
	sll	v0,t0,16
	sra	a2,v0,16
	sll	v0,a2,4
	addu	v0,v0,v1
	lbu	v1,1(v0)
	nop
	sb	v1,5(s0)
	lbu	v1,4(v0)
	nop
	sb	v1,6(s0)
	lbu	v0,0(v0)
	nop
	sb	v0,-5(s0)
	lw	v1,_svm_vh
	lb	v0,2(s0)
	lhu	v1,18(v1)
	nop
	slt	v0,v0,v1
	beq	v0,zero,.ss_vm_key_on_return_2
	addiu	v0,zero,-1
	bne	s4,zero,.ss_vm_key_on_else_2
	addiu	a0,sp,144
	sll	a0,s6,16
	sra	a0,a0,16
	sll	a1,s3,16
	sra	a1,a1,16
	jal	_SsVmKeyOff
	andi	a3,s7,$ffff
	j	.ss_vm_key_on_return
	addu	s2,v0,zero
.ss_vm_key_on_else_2:
	jal	_SsVmSelectToneAndVag
	addiu	a1,sp,16
	addu	s3,v0,zero
	andi	v0,s3,$ff
	beq	v0,zero,.ss_vm_key_on_return
	addu	s1,zero,zero
	addiu	s0,s0,17
	sll	v0,s6,16
	sra	v0,v0,16
	sw	v0,280(sp)
	andi	v1,s1,$ff
.ss_vm_key_on_loop:
	addu	v0,sp,v1
	lbu	v0,16(v0)
	nop
	sh	v0,0(s0)
	addu	v0,sp,v1
	lbu	a0,144(v0)
	lb	v1,-15(s0)
	sll	v0,a0,24
	sra	v0,v0,24
	sll	v1,v1,4
	addu	v0,v0,v1
	andi	v0,v0,$ffff
	sb	a0,-10(s0)
	lw	v1,_svm_tn
	sll	v0,v0,5
	addu	v0,v0,v1
	lbu	v1,0(v0)
	nop
	sb	v1,-7(s0)
	lbu	v1,2(v0)
	nop
	sb	v1,-9(s0)
	lbu	v1,3(v0)
	nop
	sb	v1,-8(s0)
	lbu	v1,4(v0)
	nop
	sb	v1,-6(s0)
	lbu	v1,5(v0)
	nop
	sb	v1,-5(s0)
	lbu	v0,1(v0)
	addu	a0,zero,zero
	jal	_SsVmAlloc
	sb	v0,-4(s0)
	andi	v0,v0,$ff
	lb	v1,_SsVmMaxVoice
	addu	a0,v0,zero
	slt	v1,a0,v1
	beq	v1,zero,.ss_vm_key_on_next
	sh	v0,2(s0)
	sll	v0,a0,3
	subu	v0,v0,a0
	sll	v0,v0,2
	subu	v0,v0,a0
	sll	v0,v0,1
	addiu	v1,zero,1
	lui	at,((_svm_voice+$1d)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sb	v1,((_svm_voice+$1d)-((((_svm_voice+$1d)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$2)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	zero,((_svm_voice+$2)-((((_svm_voice+$2)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$10)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	s6,((_svm_voice+$10)-((((_svm_voice+$10)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	a0,-21(s0)
	lh	v1,2(s0)
	sll	a0,a0,24
	sra	a0,a0,24
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$18)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	a0,((_svm_voice+$18)-((((_svm_voice+$18)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	a0,-15(s0)
	lh	v1,2(s0)
	sll	a0,a0,24
	sra	a0,a0,24
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$12)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	a0,((_svm_voice+$12)-((((_svm_voice+$12)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	lhu	t0,272(sp)
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$14)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	t0,((_svm_voice+$14)-((((_svm_voice+$14)+$8000)>>16)<<16))(at)
	opt	m+
	lw	t0,280(sp)
	addiu	v0,zero,33
	beq	t0,v0,.ss_vm_key_on_skip_2
	nop
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$8)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	s4,((_svm_voice+$8)-((((_svm_voice+$8)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	lbu	v1,23(s5)
	sll	v0,v0,1
	lui	at,((_svm_voice+$c)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	v1,((_svm_voice+$c)-((((_svm_voice+$c)+$8000)>>16)<<16))(at)
	opt	m+
.ss_vm_key_on_skip_2:
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$a)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sb	fp,((_svm_voice+$a)-((((_svm_voice+$a)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	a0,-10(s0)
	lh	v1,2(s0)
	sll	a0,a0,24
	sra	a0,a0,24
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$16)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	a0,((_svm_voice+$16)-((((_svm_voice+$16)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$e)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	s7,((_svm_voice+$e)-((((_svm_voice+$e)+$8000)>>16)<<16))(at)
	opt	m+
	lbu	a0,-7(s0)
	lh	v1,2(s0)
	sll	a0,a0,24
	sra	a0,a0,24
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	sll	v0,v0,1
	lui	at,((_svm_voice+$1a)+$8000)>>16
	addu	at,at,v0
	opt	m-
	sh	a0,((_svm_voice+$1a)-((((_svm_voice+$1a)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,2(s0)
	nop
	sll	v0,v1,3
	subu	v0,v0,v1
	sll	v0,v0,2
	subu	v0,v0,v1
	lhu	v1,0(s0)
	sll	v0,v0,1
	lui	at,((_svm_voice)+$8000)>>16
	addu	at,at,v0
	opt	m-
	jal	_SsVmDoAllocate
	sh	v1,((_svm_voice)-((((_svm_voice)+$8000)>>16)<<16))(at)
	opt	m+
	lh	v1,0(s0)
	addiu	v0,zero,255
	bne	v1,v0,.ss_vm_key_on_else_3
	nop
	lbu	a0,2(s0)
	jal	vmNoiseOn
	nop
	j	.ss_vm_key_on_skip_3
	nop
.ss_vm_key_on_else_3:
	jal	note2pitch
	nop
	andi	a0,s3,$ff
	jal	_SsVmKeyOnNow
	andi	a1,v0,$ffff
.ss_vm_key_on_skip_3:
	lh	v1,_svm_cur+$18
	addiu	v0,zero,1
	sllv	v0,v0,v1
	j	.ss_vm_key_on_next_2
	or	s2,s2,v0
.ss_vm_key_on_next:
	addiu	s2,zero,-1
.ss_vm_key_on_next_2:
	addiu	s1,s1,1
	andi	v0,s1,$ff
	andi	v1,s3,$ff
	sltu	v0,v0,v1
	bne	v0,zero,.ss_vm_key_on_loop
	andi	v1,s1,$ff
.ss_vm_key_on_return:
	addu	v0,s2,zero
.ss_vm_key_on_return_2:
	lw	ra,324(sp)
	lw	fp,320(sp)
	lw	s7,316(sp)
	lw	s6,312(sp)
	lw	s5,308(sp)
	lw	s4,304(sp)
	lw	s3,300(sp)
	lw	s2,296(sp)
	lw	s1,292(sp)
	lw	s0,288(sp)
	jr	ra
	addiu	sp,sp,328

_SsVmKeyOff:
	lb	v0,_SsVmMaxVoice
	addiu	sp,sp,-48
	sw	s0,16(sp)
	addu	s0,zero,zero
	sw	s1,20(sp)
	addu	s1,zero,zero
	sw	ra,44(sp)
	sw	s6,40(sp)
	sw	s5,36(sp)
	sw	s4,32(sp)
	sw	s3,28(sp)
	blez	v0,.ss_vm_key_off_loop_exit
	sw	s2,24(sp)
	addiu	s6,zero,1
	andi	s5,a3,$ffff
	sll	v0,a2,16
	sra	s4,v0,16
	sll	v0,a0,16
	sra	s3,v0,16
	sll	v0,a1,16
	sra	s2,v0,16
.ss_vm_key_off_loop:
	andi	a0,s0,$ff
	lw	v0,_snd_vmask
	sllv	v1,s6,a0
	and	v0,v0,v1
	bne	v0,zero,.ss_vm_key_off_next
	sll	v0,a0,3
	subu	v0,v0,a0
	sll	v0,v0,2
	subu	v0,v0,a0
	sll	v1,v0,1
	lui	v0,((_svm_voice+$e)+$8000)>>16
	addu	v0,v0,v1
	opt	m-
	lh	v0,((_svm_voice+$e)-((((_svm_voice+$e)+$8000)>>16)<<16))(v0)
	opt	m+
	nop
	bne	v0,s5,.ss_vm_key_off_next
	nop
	lui	v0,((_svm_voice+$14)+$8000)>>16
	addu	v0,v0,v1
	opt	m-
	lh	v0,((_svm_voice+$14)-((((_svm_voice+$14)+$8000)>>16)<<16))(v0)
	opt	m+
	nop
	bne	v0,s4,.ss_vm_key_off_next
	nop
	lui	v0,((_svm_voice+$10)+$8000)>>16
	addu	v0,v0,v1
	opt	m-
	lh	v0,((_svm_voice+$10)-((((_svm_voice+$10)+$8000)>>16)<<16))(v0)
	opt	m+
	nop
	bne	v0,s3,.ss_vm_key_off_next
	nop
	lui	v0,((_svm_voice+$18)+$8000)>>16
	addu	v0,v0,v1
	opt	m-
	lh	v0,((_svm_voice+$18)-((((_svm_voice+$18)+$8000)>>16)<<16))(v0)
	opt	m+
	nop
	bne	v0,s2,.ss_vm_key_off_next
	addiu	v0,zero,255
	lui	at,((_svm_voice)+$8000)>>16
	addu	at,at,v1
	opt	m-
	lh	v1,((_svm_voice)-((((_svm_voice)+$8000)>>16)<<16))(at)
	opt	m+
	nop
	bne	v1,v0,.ss_vm_key_off_else
	andi	v0,s0,$ff
	jal	vmNoiseOff
	addiu	s1,s1,1
	j	.ss_vm_key_off_next_2
	addiu	s0,s0,1
.ss_vm_key_off_else:
	opt	at+
	sh	v0,_svm_cur+$18
	opt	at-
	jal	_SsVmKeyOffNow
	addu	a0,zero,zero
	addiu	s1,s1,1
.ss_vm_key_off_next:
	addiu	s0,s0,1
.ss_vm_key_off_next_2:
	lb	v1,_SsVmMaxVoice
	andi	v0,s0,$ff
	slt	v0,v0,v1
	bne	v0,zero,.ss_vm_key_off_loop
	nop
.ss_vm_key_off_loop_exit:
	addu	v0,s1,zero
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

_SsVmSeKeyOn:
	addiu	sp,sp,-32
	addu	a3,a2,zero
	lw	a2,48(sp)
	lw	t0,52(sp)
	addu	t1,a1,zero
	andi	a1,a2,$ffff
	andi	v1,t0,$ffff
	bne	a1,v1,.ss_vm_se_key_on_else
	sw	ra,24(sp)
	addiu	v1,zero,64
	j	.ss_vm_se_key_on_join_3
	addu	a1,a2,zero
.ss_vm_se_key_on_else:
	sltu	v0,v1,a1
	beq	v0,zero,.ss_vm_se_key_on_else_2
	sll	v0,v1,6
	div	v0,a1
	bne	a1,zero,.ss_vm_se_key_on_skip
	nop
	break	$1c00
.ss_vm_se_key_on_skip:
	addiu	at,zero,-1
	bne	a1,at,.ss_vm_se_key_on_join
	lui	at,$8000
	bne	v0,at,.ss_vm_se_key_on_join
	nop
	break	$1800
.ss_vm_se_key_on_join:
	mflo	v0
	addu	a1,a2,zero
	j	.ss_vm_se_key_on_join_3
	addu	v1,v0,zero
.ss_vm_se_key_on_else_2:
	sll	v0,a1,6
	div	v0,v1
	bne	v1,zero,.ss_vm_se_key_on_skip_2
	nop
	break	$1c00
.ss_vm_se_key_on_skip_2:
	addiu	at,zero,-1
	bne	v1,at,.ss_vm_se_key_on_join_2
	lui	at,$8000
	bne	v0,at,.ss_vm_se_key_on_join_2
	nop
	break	$1800
.ss_vm_se_key_on_join_2:
	mflo	v0
	addu	a1,t0,zero
	addiu	v1,zero,127
	subu	v1,v1,v0
.ss_vm_se_key_on_join_3:
	andi	v0,a1,$ffff
	sw	v0,16(sp)
	andi	v0,v1,$ffff
	sll	a1,a0,16
	sll	a2,t1,16
	addiu	a0,zero,33
	sra	a1,a1,16
	sra	a2,a2,16
	andi	a3,a3,$ffff
	jal	_SsVmKeyOn
	sw	v0,20(sp)
	lw	ra,24(sp)
	addiu	sp,sp,32
	jr	ra
	nop

_SsVmSeKeyOff:
	addiu	sp,sp,-24
	sw	ra,16(sp)
	sll	v0,a0,16
	sll	v1,a1,16
	andi	a3,a2,$ffff
	addiu	a0,zero,33
	sra	a1,v0,16
	jal	_SsVmKeyOff
	sra	a2,v1,16
	lw	ra,16(sp)
	addiu	sp,sp,24
	jr	ra
	nop

KeyOnCheck:
	jr	ra
	nop
	nop
