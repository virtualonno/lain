	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastNG3,GsTMDfastG3NL,GsTMDfastG3L,GsTMDfastG3LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
ng3_vertices	macro
	lw	t2,16(a0)
	lhu	t4,20(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

g3nl_vertices	macro
	lhu	t2,10(a0)
	lhu	t3,14(a0)
	lhu	t4,18(a0)
	sll	t2,t2,3
	sll	t3,t3,3
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

g3_color	macro
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lw	t2,4(a0)
	sll	t3,t3,25
	or	t2,t2,t3
	mtc2	t2,C2_RGB0
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

; Prepend one POLY_G3 to the ordering table, preserving the tag's high byte.
link_g3	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0600
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

g3_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,16(packet)
	swc2	C2_SXY2,24(packet)
	endm

g3_colors	macro packet
	swc2	C2_RGB0,4(packet)
	swc2	C2_RGB1,12(packet)
	swc2	C2_RGB2,20(packet)
	endm

g3l_indices	macro
	lw	t2,8(a0)
	lw	t3,12(a0)
	lw	t4,16(a0)
	endm

; Each packed index pair names one vertex and its lighting normal.
load_vertex	macro indices,xy,z
	srl	t5,indices,16
	sll	t5,t5,3
	addu	t5,t5,a1
	lwc2	xy,0(t5)
	lwc2	z,4(t5)
	endm

normal_addr	macro indices
	sll	indices,indices,16
	srl	indices,indices,13
	addu	indices,indices,a2
	endm

g3l_transform	macro skip
	load_vertex t2,C2_VXY0,C2_VZ0
	load_vertex t3,C2_VXY1,C2_VZ1
	load_vertex t4,C2_VXY2,C2_VZ2
	nop
	wRTPT
	normal_addr t2
	normal_addr t3
	normal_addr t4
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	NCLIP
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	lw	t5,4(a0)
	sll	t6,t6,25
	or	t5,t5,t6
	mtc2	t5,C2_RGB
	mfc2	v0,C2_MAC0
	nop
	blez	v0,skip
	nop
	wAVSZ3
	lwc2	C2_VXY0,0(t2)
	lwc2	C2_VZ0,4(t2)
	lwc2	C2_VXY1,0(t3)
	lwc2	C2_VZ1,4(t3)
	lwc2	C2_VXY2,0(t4)
	lwc2	C2_VZ2,4(t4)
	mfc2	t6,C2_OTZ
	nop
	endm

	section	.text

GsTMDfastNG3:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,ng3_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	ng3_vertices
ng3_loop:
	project3
	g3_color
	lwc2	C2_RGB1,8(a0)
	lwc2	C2_RGB2,12(a0)
	addiu	a0,a0,24
	ng3_vertices
	clip3 ng3_next
	mfc2	t6,C2_OTZ
	g3_colors a2
	g3_xy a2
	link_g3 a2
	addiu	a2,a2,28
ng3_next:
	addiu	a3,a3,-1
	bne	a3,zero,ng3_loop
	nop
ng3_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastG3NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g3nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	g3nl_vertices
g3nl_loop:
	project3
	g3_color
	addiu	a0,a0,20
	g3nl_vertices
	clip3 g3nl_next
	mfc2	t6,C2_OTZ
	nop
	link_g3 a3
	swc2	C2_RGB0,4(a3)
	swc2	C2_RGB0,12(a3)
	swc2	C2_RGB0,20(a3)
	g3_xy a3
	addiu	a3,a3,28
g3nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,g3nl_loop
	nop
g3nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastG3L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g3l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
g3l_indices_next:
	g3l_indices
g3l_loop:
	g3l_transform g3l_skip
	wNCCT
	addiu	a0,a0,20
	link_g3 a3
	g3l_indices
	nop
	g3_xy a3
	g3_colors a3
	addiu	a3,a3,28
	addiu	t8,t8,-1
	bne	t8,zero,g3l_loop
	nop
	beq	zero,zero,g3l_return
	nop
g3l_skip:
	addiu	a0,a0,20
	addiu	t8,t8,-1
	bne	t8,zero,g3l_indices_next
	nop
g3l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastG3LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g3lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
g3lfg_indices_next:
	g3l_indices
g3lfg_loop:
	g3l_transform g3lfg_skip
	wNCDT
	addiu	a0,a0,20
	link_g3 a3
	g3l_indices
	nop
	g3_xy a3
	g3_colors a3
	addiu	a3,a3,28
	addiu	t8,t8,-1
	bne	t8,zero,g3lfg_loop
	nop
	beq	zero,zero,g3lfg_return
	nop
g3lfg_skip:
	addiu	a0,a0,20
	addiu	t8,t8,-1
	bne	t8,zero,g3lfg_indices_next
	nop
g3lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
	nop
