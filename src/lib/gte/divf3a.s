; LIBGTE DIVF3A
	opt	c+,at-,m-
#include <gtereg.h>

wRTPT	macro
	dw	$4a280030
	endm

	xdef	RCpolyF3
	xdef	RCpolyF3A

	section	.text

RCpolyF3:
	addi	a3,a1,96
	addiu	a2,zero,0

RCpolyF3A:
	lw	t0,72(a3)
	lw	t1,76(a3)
	lw	t2,80(a3)
	cfc2	t9,C2_H
	lw	t4,20(t0)
	lw	t5,20(t1)
	lw	t6,20(t2)
	sra	t8,t9,1
	sltu	at,t4,t8
	beq	at,zero,.r_cpoly_f3_a_else
	sltu	at,t5,t8
	beq	at,zero,.r_cpoly_f3_a_else
	sltu	at,t6,t8
	beq	at,zero,.r_cpoly_f3_a_else
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_else:
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
	slt	at,t8,t4
	beq	at,zero,.r_cpoly_f3_a_else_2
	slt	at,t8,t5
	beq	at,zero,.r_cpoly_f3_a_else_2
	slt	at,t8,t6
	beq	at,zero,.r_cpoly_f3_a_else_2
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_else_2:
	subu	t8,t9,v0
	slt	at,t4,t8
	beq	at,zero,.r_cpoly_f3_a_else_3
	slt	at,t5,t8
	beq	at,zero,.r_cpoly_f3_a_else_3
	slt	at,t6,t8
	beq	at,zero,.r_cpoly_f3_a_else_3
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_else_3:
	cfc2	t9,C2_OFY
	lh	t4,18(t0)
	lh	t5,18(t1)
	lh	t6,18(t2)
	sra	t9,t9,16
	addu	t8,t9,v1
	slt	at,t8,t4
	beq	at,zero,.r_cpoly_f3_a_else_4
	slt	at,t8,t5
	beq	at,zero,.r_cpoly_f3_a_else_4
	slt	at,t8,t6
	beq	at,zero,.r_cpoly_f3_a_else_4
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_else_4:
	subu	t8,t9,v1
	slt	at,t4,t8
	beq	at,zero,.r_cpoly_f3_a_else_5
	slt	at,t5,t8
	beq	at,zero,.r_cpoly_f3_a_else_5
	slt	at,t6,t8
	beq	at,zero,.r_cpoly_f3_a_else_5
	nop
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_else_5:
	lh	t4,0(t0)
	lh	t5,0(t1)
	lh	t6,0(t2)
	add	t7,t4,t5
	add	t8,t5,t6
	add	t9,t6,t4
	sra	t7,t7,1
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t7,0(a3)
	sh	t8,24(a3)
	sh	t9,48(a3)
	lh	t4,2(t0)
	lh	t5,2(t1)
	lh	t6,2(t2)
	add	t7,t4,t5
	add	t8,t5,t6
	add	t9,t6,t4
	sra	t7,t7,1
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t7,2(a3)
	sh	t8,26(a3)
	sh	t9,50(a3)
	lh	t4,4(t0)
	lh	t5,4(t1)
	lh	t6,4(t2)
	add	t7,t4,t5
	add	t8,t5,t6
	add	t9,t6,t4
	sra	t7,t7,1
	sra	t8,t8,1
	sra	t9,t9,1
	sh	t7,4(a3)
	sh	t8,28(a3)
	sh	t9,52(a3)
	lwc2	C2_VXY0,0(a3)
	lwc2	C2_VZ0,4(a3)
	lwc2	C2_VXY1,24(a3)
	lwc2	C2_VZ1,28(a3)
	lwc2	C2_VXY2,48(a3)
	lwc2	C2_VZ2,52(a3)
	nop
	wRTPT
	lw	t4,0(a1)
	addiu	a2,a2,1
	bne	t4,a2,.r_cpoly_f3_a_else_6
	nop
	addu	v1,ra,zero
	lw	t0,76(a3)
	addiu	t1,a3,24
	addiu	t2,a3,0
	swc2	C2_SXY0,16(a3)
	swc2	C2_SXY1,40(a3)
	swc2	C2_SXY2,64(a3)
	jal	.r_cpoly_f3_a_return_2
	nop
	addiu	t0,a3,0
	addiu	t1,a3,24
	addiu	t2,a3,48
	jal	.r_cpoly_f3_a_return_2
	nop
	lw	t0,72(a3)
	addiu	t1,a3,0
	addiu	t2,a3,48
	jal	.r_cpoly_f3_a_return_2
	nop
	lw	t0,80(a3)
	addiu	t1,a3,48
	addiu	t2,a3,24
	jal	.r_cpoly_f3_a_return_2
	nop
	addu	ra,v1,zero
	addiu	a2,a2,-1
	beq	zero,zero,.r_cpoly_f3_a_return
	nop
.r_cpoly_f3_a_else_6:
	addiu	a3,a3,88
	sw	ra,84(a3)
	lw	t4,-16(a3)
	addiu	t5,a3,-88
	addiu	t6,a3,-40
	sw	t4,72(a3)
	sw	t5,76(a3)
	sw	t6,80(a3)
	swc2	C2_SZ1,-68(a3)
	swc2	C2_SZ2,-44(a3)
	swc2	C2_SZ3,-20(a3)
	swc2	C2_SXY0,-72(a3)
	swc2	C2_SXY1,-48(a3)
	swc2	C2_SXY2,-24(a3)
	jal	RCpolyF3A
	nop
	lw	t4,-12(a3)
	addiu	t5,a3,-64
	addiu	t6,a3,-88
	sw	t4,72(a3)
	sw	t5,76(a3)
	sw	t6,80(a3)
	jal	RCpolyF3A
	nop
	lw	t4,-8(a3)
	addiu	t5,a3,-40
	addiu	t6,a3,-64
	sw	t4,72(a3)
	sw	t5,76(a3)
	sw	t6,80(a3)
	jal	RCpolyF3A
	nop
	addiu	t4,a3,-88
	addiu	t5,a3,-64
	addiu	t6,a3,-40
	sw	t4,72(a3)
	sw	t5,76(a3)
	sw	t6,80(a3)
	jal	RCpolyF3A
	nop
	lw	ra,84(a3)
	addiu	a3,a3,-88
	addiu	a2,a2,-1
.r_cpoly_f3_a_return:
	addu	v0,a0,zero
	jr	ra
	nop
.r_cpoly_f3_a_return_2:
	lw	t8,16(a1)
	lw	t4,16(t0)
	lw	t5,16(t1)
	lw	t6,16(t2)
	sw	t4,8(a0)
	sw	t5,12(a0)
	sw	t6,16(a0)
	sw	t8,4(a0)
	lw	t4,20(a1)
	sll	t9,a0,8
	srl	t9,t9,8
	lw	t8,0(t4)
	sw	t9,0(t4)
	lui	t6,$400
	or	t8,t8,t6
	sw	t8,0(a0)
	addiu	a0,a0,20
	jr	ra
	nop
	nop
