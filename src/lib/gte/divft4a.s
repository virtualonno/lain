; LIBGTE DIVFT4A
	opt	c+,at-,m-
#include <gtereg.h>

wRTPT	macro
	dw	$4a280030
	endm

	xdef	RCpolyFT4
	xdef	RCpolyFT4A

	section	.text

RCpolyFT4:
	addi	a3,a1,120
	addiu	a2,zero,0

RCpolyFT4A:
	lw	t0,120(a3)
	lw	t1,124(a3)
	lw	t2,128(a3)
	lw	t3,132(a3)
	cfc2	t9,C2_H
	lw	t4,20(t0)
	lw	t5,20(t1)
	lw	t6,20(t2)
	lw	t7,20(t3)
	sra	t8,t9,1
	sltu	at,t4,t8
	beq	at,zero,.r_cpoly_ft4_a_else
	sltu	at,t5,t8
	beq	at,zero,.r_cpoly_ft4_a_else
	sltu	at,t6,t8
	beq	at,zero,.r_cpoly_ft4_a_else
	sltu	at,t7,t8
	beq	at,zero,.r_cpoly_ft4_a_else
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_else:
	cfc2	t9,C2_OFX
	lw	v0,4(a1)
	lw	v1,8(a1)
	sra	t9,t9,16
	srl	v0,v0,1
	srl	v1,v1,1
	addu	t8,t9,v0
	lh	t4,16(t0)
	lh	t5,16(t1)
	lh	t6,16(t2)
	lh	t7,16(t3)
	slt	at,t8,t4
	beq	at,zero,.r_cpoly_ft4_a_else_2
	slt	at,t8,t5
	beq	at,zero,.r_cpoly_ft4_a_else_2
	slt	at,t8,t6
	beq	at,zero,.r_cpoly_ft4_a_else_2
	slt	at,t8,t7
	beq	at,zero,.r_cpoly_ft4_a_else_2
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_else_2:
	subu	t8,t9,v0
	slt	at,t4,t8
	beq	at,zero,.r_cpoly_ft4_a_else_3
	slt	at,t5,t8
	beq	at,zero,.r_cpoly_ft4_a_else_3
	slt	at,t6,t8
	beq	at,zero,.r_cpoly_ft4_a_else_3
	slt	at,t7,t8
	beq	at,zero,.r_cpoly_ft4_a_else_3
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_else_3:
	cfc2	t9,C2_OFY
	lh	t4,18(t0)
	lh	t5,18(t1)
	lh	t6,18(t2)
	lh	t7,18(t3)
	sra	t9,t9,16
	addu	t8,t9,v1
	slt	at,t8,t4
	beq	at,zero,.r_cpoly_ft4_a_else_4
	slt	at,t8,t5
	beq	at,zero,.r_cpoly_ft4_a_else_4
	slt	at,t8,t6
	beq	at,zero,.r_cpoly_ft4_a_else_4
	slt	at,t8,t7
	beq	at,zero,.r_cpoly_ft4_a_else_4
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_else_4:
	subu	t8,t9,v1
	slt	at,t4,t8
	beq	at,zero,.r_cpoly_ft4_a_else_5
	slt	at,t5,t8
	beq	at,zero,.r_cpoly_ft4_a_else_5
	slt	at,t6,t8
	beq	at,zero,.r_cpoly_ft4_a_else_5
	slt	at,t7,t8
	beq	at,zero,.r_cpoly_ft4_a_else_5
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_else_5:
	lh	t4,0(t0)
	lh	t5,0(t1)
	lh	t6,0(t2)
	lh	t7,0(t3)
	add	t9,t4,t6
	add	t8,t7,t5
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t8,48(a3)
	sh	t9,24(a3)
	add	t8,t4,t5
	add	t9,t7,t6
	add	v1,t8,t9
	sra	t8,t8,1
	sra	t9,t9,1
	sra	v1,v1,2
	sh	t8,0(a3)
	sh	t9,72(a3)
	sh	v1,96(a3)
	lh	t4,2(t0)
	lh	t5,2(t1)
	lh	t6,2(t2)
	lh	t7,2(t3)
	add	t9,t4,t6
	add	t8,t7,t5
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t8,50(a3)
	sh	t9,26(a3)
	add	t8,t4,t5
	add	t9,t7,t6
	add	v1,t8,t9
	sra	t8,t8,1
	sra	t9,t9,1
	sra	v1,v1,2
	sh	t8,2(a3)
	sh	t9,74(a3)
	sh	v1,98(a3)
	lh	t4,4(t0)
	lh	t5,4(t1)
	lh	t6,4(t2)
	lh	t7,4(t3)
	add	t9,t4,t6
	add	t8,t7,t5
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t8,52(a3)
	sh	t9,28(a3)
	add	t8,t4,t5
	add	t9,t7,t6
	add	v1,t8,t9
	sra	t8,t8,1
	sra	t9,t9,1
	sra	v1,v1,2
	sh	t8,4(a3)
	sh	t9,76(a3)
	sh	v1,100(a3)
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	lwc2	C2_VXY1,24(a3)
	lwc2	C2_VZ1,28(a3)
	lwc2	C2_VXY2,96(a3)
	lwc2	C2_VZ2,100(a3)
	lbu	t4,8(t0)
	wRTPT
	lbu	t5,8(t1)
	lbu	t6,8(t2)
	lbu	t7,8(t3)
	add	t9,t4,t6
	add	t8,t7,t5
	sra	t8,t8,1
	sra	t9,t9,1
	sb	t8,56(a3)
	sb	t9,32(a3)
	add	t8,t4,t5
	add	t9,t7,t6
	add	v1,t8,t9
	sra	t8,t8,1
	sra	t9,t9,1
	sra	v1,v1,2
	sb	t8,8(a3)
	sb	t9,80(a3)
	sb	v1,104(a3)
	swc2	C2_SXY0,16(a3)
	swc2	C2_SXY1,40(a3)
	swc2	C2_SXY2,112(a3)
	swc2	C2_SZ1,20(a3)
	swc2	C2_SZ2,44(a3)
	swc2	C2_SZ3,116(a3)
	lwc2	C2_VXY0,48(a3)
	lwc2	C2_VZ0,52(a3)
	lwc2	C2_VXY1,72(a3)
	lwc2	C2_VZ1,76(a3)
	lbu	t4,9(t0)
	wRTPT
	lbu	t5,9(t1)
	lbu	t6,9(t2)
	lbu	t7,9(t3)
	add	t9,t4,t6
	add	t8,t7,t5
	sra	t8,t8,1
	sra	t9,t9,1
	sb	t8,57(a3)
	sb	t9,33(a3)
	add	t8,t4,t5
	add	t9,t7,t6
	add	v1,t8,t9
	sra	t8,t8,1
	sra	t9,t9,1
	sra	v1,v1,2
	sb	t8,9(a3)
	sb	t9,81(a3)
	sb	v1,105(a3)
	lw	t4,0(a1)
	addiu	a2,a2,1
	bne	t4,a2,.r_cpoly_ft4_a_else_6
	nop
	addu	v1,ra,zero
	lw	t0,120(a3)
	addiu	t1,a3,0
	addiu	t2,a3,24
	addiu	t3,a3,96
	swc2	C2_SXY0,64(a3)
	swc2	C2_SXY1,88(a3)
	jal	.r_cpoly_ft4_a_helper
	nop
	lw	t0,124(a3)
	addiu	t1,a3,48
	addiu	t2,a3,0
	addiu	t3,a3,96
	jal	.r_cpoly_ft4_a_helper
	nop
	lw	t0,128(a3)
	addiu	t1,a3,24
	addiu	t2,a3,72
	addiu	t3,a3,96
	jal	.r_cpoly_ft4_a_helper
	nop
	lw	t0,132(a3)
	addiu	t1,a3,72
	addiu	t2,a3,48
	addiu	t3,a3,96
	jal	.r_cpoly_ft4_a_helper
	nop
	addu	ra,v1,zero
	addiu	a2,a2,-1
	beq	zero,zero,.r_cpoly_ft4_a_return
	nop
.r_cpoly_ft4_a_else_6:
	addiu	a3,a3,140
	sw	ra,136(a3)
	lw	t4,-20(a3)
	addiu	t5,a3,-140
	addiu	t6,a3,-116
	addiu	t7,a3,-44
	sw	t4,120(a3)
	sw	t5,124(a3)
	sw	t6,128(a3)
	sw	t7,132(a3)
	swc2	C2_SXY0,-76(a3)
	swc2	C2_SXY1,-52(a3)
	swc2	C2_SZ1,-72(a3)
	swc2	C2_SZ2,-48(a3)
	jal	RCpolyFT4A
	nop
	lw	t4,-16(a3)
	addiu	t5,a3,-92
	addiu	t6,a3,-140
	addiu	t7,a3,-44
	sw	t4,120(a3)
	sw	t5,124(a3)
	sw	t6,128(a3)
	sw	t7,132(a3)
	jal	RCpolyFT4A
	nop
	lw	t4,-12(a3)
	addiu	t5,a3,-116
	addiu	t6,a3,-68
	addiu	t7,a3,-44
	sw	t4,120(a3)
	sw	t5,124(a3)
	sw	t6,128(a3)
	sw	t7,132(a3)
	jal	RCpolyFT4A
	nop
	lw	t4,-8(a3)
	addiu	t5,a3,-68
	addiu	t6,a3,-92
	addiu	t7,a3,-44
	sw	t4,120(a3)
	sw	t5,124(a3)
	sw	t6,128(a3)
	sw	t7,132(a3)
	jal	RCpolyFT4A
	nop
	lw	ra,136(a3)
	addiu	a3,a3,-140
	addiu	a2,a2,-1
.r_cpoly_ft4_a_return:
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_ft4_a_helper:
	lhu	t4,12(a1)
	lhu	t5,8(t0)
	sll	t4,t4,16
	addu	t5,t5,t4
	lhu	t4,14(a1)
	lhu	t6,8(t1)
	sll	t4,t4,16
	addu	t6,t6,t4
	lhu	t7,8(t2)
	lhu	t8,8(t3)
	lw	t4,16(a1)
	sw	t5,12(a0)
	sw	t6,20(a0)
	sw	t7,28(a0)
	sw	t8,36(a0)
	sw	t4,4(a0)
	lw	t4,16(t0)
	lw	t5,16(t1)
	lw	t6,16(t2)
	lw	t7,16(t3)
	sw	t4,8(a0)
	sw	t5,16(a0)
	sw	t6,24(a0)
	sw	t7,32(a0)
	lw	t4,20(a1)
	sll	t9,a0,8
	srl	t9,t9,8
	lw	t8,0(t4)
	sw	t9,0(t4)
	lui	t6,$900
	or	t8,t8,t6
	sw	t8,0(a0)
	addiu	a0,a0,40
	jr	ra
	nop
	nop
