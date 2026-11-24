	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastTNF3,GsTMDfastTF3NL,GsTMDfastTF3L,GsTMDfastTF3LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
tnf3_vertices	macro
	lw	t2,20(a0)
	lhu	t4,24(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

tf3nl_vertices	macro
	lw	t3,20(a0)
	lhu	t2,18(a0)
	srl	t4,t3,16
	sll	t3,t3,16
	srl	t3,t3,13
	sll	t2,t2,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

project3	macro
	lwc2	C2_VXY0,0(t2)
	lwc2	C2_VZ0,4(t2)
	lwc2	C2_VXY1,0(t3)
	lwc2	C2_VZ1,4(t3)
	lwc2	C2_VXY2,0(t4)
	lwc2	C2_VZ2,4(t4)
	nop
	wRTPT
	endm

; The model holds a mode byte separate from the color and UV words.
tf3_mode	macro
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lb	t2,3(a0)
	sll	t3,t3,1
	or	t2,t2,t3
	endm

tf3_neutral_color	macro
	lui	v0,$0080
	ori	v0,v0,$8080
	tf3_mode
	sll	t2,t2,24
	or	v0,v0,t2
	endm

tf3_uv	macro packet
	lw	t2,4(a0)
	lw	t3,8(a0)
	lw	t4,12(a0)
	sw	t2,12(packet)
	sw	t3,20(packet)
	sw	t4,28(packet)
	endm

clip3	macro skip
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	NCLIP
	mfc2	v0,C2_MAC0
	nop
	blez	v0,skip
	nop
	wAVSZ3
	endm

; Prepend one POLY_FT3 to the ordering table, preserving the tag's high byte.
link_tf3	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0700
	sll	v1,t5,8
	srl	v1,v1,8
	or	v0,v0,v1
	sw	v0,0(packet)
	xor	v0,packet,t5
	sll	v1,v0,8
	srl	v0,v1,8
	xor	v1,v0,t5
	sw	v1,0(t6)
	endm

tf3_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,16(packet)
	swc2	C2_SXY2,24(packet)
	endm

tf3l_first_vertex	macro
	lw	t2,16(a0)
	lw	t3,20(a0)
	srl	t4,t2,16
	sll	t4,t4,3
	addu	t4,t4,a1
	endm

tf3l_transform	macro
	lwc2	C2_VXY0,0(t4)
	lwc2	C2_VZ0,4(t4)
	sll	t5,t3,16
	srl	t5,t5,13
	addu	t5,t5,a1
	lwc2	C2_VXY1,0(t5)
	lwc2	C2_VZ1,4(t5)
	srl	t5,t3,16
	sll	t5,t5,3
	addu	t5,t5,a1
	lwc2	C2_VXY2,0(t5)
	lwc2	C2_VZ2,4(t5)
	nop
	wRTPT
	sll	t5,t2,16
	srl	t5,t5,13
	addu	t5,t5,a2
	tf3_neutral_color
	mtc2	v0,C2_RGB
	tf3_uv a3
	addiu	a0,a0,24
	tf3l_first_vertex
	endm

tf3l_normal_depth	macro
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

	section	.text

GsTMDfastTNF3:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,tnf3_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tnf3_vertices
tnf3_loop:
	project3
	lw	v0,16(a0)
	tf3_mode
	sll	v0,v0,8
	srl	v0,v0,8
	srl	t2,t2,1
	sll	t2,t2,25
	or	t2,t2,v0
	mtc2	t2,C2_RGB2
	tf3_uv a2
	addiu	a0,a0,28
	tnf3_vertices
	clip3 tnf3_next
	mfc2	t6,C2_OTZ
	nop
	link_tf3 a2
	swc2	C2_RGB2,4(a2)
	tf3_xy a2
	addiu	a2,a2,32
tnf3_next:
	addiu	a3,a3,-1
	bne	a3,zero,tnf3_loop
	nop
tnf3_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastTF3NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf3nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf3nl_vertices
tf3nl_loop:
	project3
	tf3_neutral_color
	tf3_uv a3
	sw	v0,4(a3)
	addiu	a0,a0,24
	tf3nl_vertices
	clip3 tf3nl_next
	mfc2	t6,C2_OTZ
	nop
	link_tf3 a3
	tf3_xy a3
	addiu	a3,a3,32
tf3nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf3nl_loop
	nop
tf3nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTF3L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf3l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf3l_first_vertex
tf3l_loop:
	tf3l_transform
	clip3 tf3l_next
	tf3l_normal_depth
	wNCCS
	link_tf3 a3
	tf3_xy a3
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,32
tf3l_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf3l_loop
	nop
tf3l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTF3LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf3lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf3l_first_vertex
tf3lfg_loop:
	tf3l_transform
	clip3 tf3lfg_next
	tf3l_normal_depth
	wNCDS
	link_tf3 a3
	tf3_xy a3
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,32
tf3lfg_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf3lfg_loop
	nop
tf3lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
