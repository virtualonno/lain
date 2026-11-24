	opt	c+,at-,m-
#include <gtereg.h>
	include	"gte_word.inc"

	xdef	ApplyRotMatrixLV

	section	.text

ApplyRotMatrixLV:
	lw	t0,0(a0)
	lw	t1,4(a0)
	lw	t2,8(a0)
	; Split each signed long for the GTE's signed IR inputs.
	bgez	t0,.x_positive
	nop
	subu	t0,zero,t0
	sra	t3,t0,15
	subu	t3,zero,t3
	andi	t0,t0,$7fff
	beq	zero,zero,.split_y
	subu	t0,zero,t0
.x_positive:
	sra	t3,t0,15
	andi	t0,t0,$7fff
.split_y:
	bgez	t1,.y_positive
	nop
	subu	t1,zero,t1
	sra	t4,t1,15
	subu	t4,zero,t4
	andi	t1,t1,$7fff
	beq	zero,zero,.split_z
	subu	t1,zero,t1
.y_positive:
	sra	t4,t1,15
	andi	t1,t1,$7fff
.split_z:
	bgez	t2,.z_positive
	nop
	subu	t2,zero,t2
	sra	t5,t2,15
	subu	t5,zero,t5
	andi	t2,t2,$7fff
	beq	zero,zero,.rotate_high
	subu	t2,zero,t2
.z_positive:
	sra	t5,t2,15
	andi	t2,t2,$7fff
.rotate_high:
	mtc2	t3,C2_IR1
	mtc2	t4,C2_IR2
	mtc2	t5,C2_IR3
	nop
	wMVMVA 0,0,3,3,0
	mfc2	t3,C2_MAC1
	mfc2	t4,C2_MAC2
	mfc2	t5,C2_MAC3
	mtc2	t0,C2_IR1
	mtc2	t1,C2_IR2
	mtc2	t2,C2_IR3
	nop
	wMVMVA 1,0,3,3,0
	; The low-part product is already shifted by 12 in the GTE;
	; in those units the unshifted high-part product weighs 8.
	bgez	t3,.high_x_positive
	nop
	subu	t3,zero,t3
	sll	t3,t3,3
	beq	zero,zero,.scale_y
	subu	t3,zero,t3
.high_x_positive:
	sll	t3,t3,3
.scale_y:
	bgez	t4,.high_y_positive
	nop
	subu	t4,zero,t4
	sll	t4,t4,3
	beq	zero,zero,.scale_z
	subu	t4,zero,t4
.high_y_positive:
	sll	t4,t4,3
.scale_z:
	bgez	t5,.high_z_positive
	nop
	subu	t5,zero,t5
	sll	t5,t5,3
	beq	zero,zero,.combine
	subu	t5,zero,t5
.high_z_positive:
	sll	t5,t5,3
.combine:
	mfc2	t0,C2_MAC1
	mfc2	t1,C2_MAC2
	mfc2	t2,C2_MAC3
	addu	t0,t0,t3
	addu	t1,t1,t4
	addu	t2,t2,t5
	sw	t0,0(a1)
	sw	t1,4(a1)
	sw	t2,8(a1)
	jr	ra
	addu	v0,a1,zero
	nop
	nop
