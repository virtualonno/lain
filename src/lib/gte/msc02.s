	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"INLINE_A.H"
	include	"gte_word.inc"

	xdef	InvSquareRoot
	xdef	VectorNormal
	xdef	VectorNormalS
	xdef	VectorNormalSS
	xdef	MatrixNormal

	section	.text

InvSquareRoot:
	mtc2	a0,C2_LZCS
	nop
	nop
	mfc2	v0,C2_LZCR
	addiu	at,zero,32
	beq	v0,at,.L_inv_err
	nop
	beq	v0,zero,.L_inv_err
	nop
	andi	t0,v0,$1
	addiu	t2,zero,-2
	and	t2,v0,t2
	addiu	t1,zero,31
	sub	t1,t1,t2
	sra	t1,t1,1
	addi	t3,t2,-24
	bltz	t3,.L_inv_shr
	nop
	sllv	t4,a0,t3
	beq	zero,zero,.L_inv_lookup
.L_inv_shr:
	addiu	t3,zero,24
	sub	t3,t3,t2
	srav	t4,a0,t3
.L_inv_lookup:
	addi	t4,t4,-64
	sll	t4,t4,1
	lui	t5,HI(ISQRT_tbl)
	addu	t5,t5,t4
	lh	t5,LO(ISQRT_tbl)(t5)
	sw	t1,0(a2)
	sw	t5,0(a1)
	addiu	v0,zero,1
	jr	ra
	nop
.L_inv_err:
	jr	ra
	addiu	v0,zero,-1

VectorNormalS:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	beq	zero,zero,.L_vec_norm_s_join
	addu	a3,ra,zero

VectorNormal:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	addu	a3,ra,zero
	jal	.L_vec_norm_core
	nop
	addu	ra,a3,zero
	sw	t0,0(a1)
	sw	t1,4(a1)
	jr	ra
	sw	t2,8(a1)

VectorNormalSS:
	lh	t0,0(a0)
	lh	t1,2(a0)
	lh	t2,4(a0)
	nop
	addu	a3,ra,zero
.L_vec_norm_s_join:
	jal	.L_vec_norm_core
	nop
	addu	ra,a3,zero
	sh	t0,0(a1)
	sh	t1,2(a1)
	jr	ra
	sh	t2,4(a1)

.L_vec_norm_core:
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wSQR	0
	mfc2	t3,C2_MAC1
	mfc2	t4,C2_MAC2
	mfc2	t5,C2_MAC3
	add	t3,t3,t4
	add	v0,t3,t5
	mtc2	v0,C2_LZCS
	nop
	nop
	mfc2	v1,C2_LZCR
	addiu	at,zero,-2
	and	v1,v1,at
	addiu	t6,zero,31
	sub	t6,t6,v1
	addi	t3,v1,-24
	bltz	t3,.L_norm_shr
	sra	t6,t6,1
	beq	zero,zero,.L_norm_lookup
	sllv	t4,v0,t3
.L_norm_shr:
	addiu	t3,zero,24
	sub	t3,t3,v1
	srav	t4,v0,t3
.L_norm_lookup:
	addi	t4,t4,-64
	sll	t4,t4,1
	lui	t5,HI(ISQRT_tbl)
	addu	t5,t5,t4
	lh	t5,LO(ISQRT_tbl)(t5)
	nop
	mtc2	t5,C2_IR0
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nGPF	0
	mfc2	t0,C2_MAC1
	mfc2	t1,C2_MAC2
	mfc2	t2,C2_MAC3
	srav	t0,t0,t6
	srav	t1,t1,t6
	srav	t2,t2,t6
	jr	ra
	nop

MatrixNormal:
	lh	t0,0(a0)
	lh	t1,2(a0)
	lh	t2,4(a0)
	lh	t3,6(a0)
	lh	t4,8(a0)
	lh	t5,10(a0)
	cfc2	v0,C2_R11R12
	cfc2	v1,C2_R22R23
	cfc2	a2,C2_R33
	ctc2	t0,C2_R11R12
	ctc2	t1,C2_R22R23
	ctc2	t2,C2_R33
	mtc2	t5,C2_IR3
	mtc2	t3,C2_IR1
	mtc2	t4,C2_IR2
	nop
	wOP	1
	mfc2	t7,C2_MAC1
	mfc2	t8,C2_MAC2
	mfc2	t9,C2_MAC3
	ctc2	t3,C2_R11R12
	ctc2	t4,C2_R22R23
	ctc2	t5,C2_R33
	nop
	wOP	1
	mtc2	t3,C2_VXY0
	mtc2	t4,C2_VZ0
	mtc2	t5,C2_VXY1
	mfc2	t0,C2_MAC1
	mfc2	t1,C2_MAC2
	mfc2	t2,C2_MAC3
	ctc2	v0,C2_R11R12
	ctc2	v1,C2_R22R23
	ctc2	a2,C2_R33
	addu	a3,ra,zero
	jal	.L_vec_norm_core
	nop
	sh	t0,0(a1)
	sh	t1,2(a1)
	sh	t2,4(a1)
	mfc2	t0,C2_VXY0
	mfc2	t1,C2_VZ0
	mfc2	t2,C2_VXY1
	jal	.L_vec_norm_core
	nop
	sh	t0,6(a1)
	sh	t1,8(a1)
	sh	t2,10(a1)
	addu	t0,t7,zero
	addu	t1,t8,zero
	jal	.L_vec_norm_core
	addu	t2,t9,zero
	addu	ra,a3,zero
	sh	t0,12(a1)
	sh	t1,14(a1)
	jr	ra
	sh	t2,16(a1)
	nop

	section	.data
	dw	0,0,0,0,0
; 1/sqrt(m/64) in Q12 for m = 64..255
ISQRT_tbl:
	dh	$1000,$0fe0,$0fc1,$0fa3,$0f85,$0f68,$0f4c,$0f30
	dh	$0f15,$0efb,$0ee1,$0ec7,$0eae,$0e96,$0e7e,$0e66
	dh	$0e4f,$0e38,$0e22,$0e0c,$0df7,$0de2,$0dcd,$0db9
	dh	$0da5,$0d91,$0d7e,$0d6b,$0d58,$0d45,$0d33,$0d21
	dh	$0d10,$0cff,$0cee,$0cdd,$0ccc,$0cbc,$0cac,$0c9c
	dh	$0c8d,$0c7d,$0c6e,$0c5f,$0c51,$0c42,$0c34,$0c26
	dh	$0c18,$0c0a,$0bfd,$0bef,$0be2,$0bd5,$0bc8,$0bbb
	dh	$0baf,$0ba2,$0b96,$0b8a,$0b7e,$0b72,$0b67,$0b5b
	dh	$0b50,$0b45,$0b39,$0b2e,$0b24,$0b19,$0b0e,$0b04
	dh	$0af9,$0aef,$0ae5,$0adb,$0ad1,$0ac7,$0abd,$0ab4
	dh	$0aaa,$0aa1,$0a97,$0a8e,$0a85,$0a7c,$0a73,$0a6a
	dh	$0a61,$0a59,$0a50,$0a47,$0a3f,$0a37,$0a2e,$0a26
	dh	$0a1e,$0a16,$0a0e,$0a06,$09fe,$09f6,$09ef,$09e7
	dh	$09e0,$09d8,$09d1,$09c9,$09c2,$09bb,$09b4,$09ad
	dh	$09a5,$099e,$0998,$0991,$098a,$0983,$097c,$0976
	dh	$096f,$0969,$0962,$095c,$0955,$094f,$0949,$0943
	dh	$093c,$0936,$0930,$092a,$0924,$091e,$0918,$0912
	dh	$090d,$0907,$0901,$08fb,$08f6,$08f0,$08eb,$08e5
	dh	$08e0,$08da,$08d5,$08cf,$08ca,$08c5,$08bf,$08ba
	dh	$08b5,$08b0,$08ab,$08a6,$08a1,$089c,$0897,$0892
	dh	$088d,$0888,$0883,$087e,$087a,$0875,$0870,$086b
	dh	$0867,$0862,$085e,$0859,$0855,$0850,$084c,$0847
	dh	$0843,$083e,$083a,$0836,$0831,$082d,$0829,$0824
	dh	$0820,$081c,$0818,$0814,$0810,$080c,$0808,$0804
	dw	0,0,0
