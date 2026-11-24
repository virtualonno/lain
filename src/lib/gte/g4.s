	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastNG4,GsTMDfastG4NL,GsTMDfastG4L,GsTMDfastG4LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
ng4_vertices	macro
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

g4nl_vertices	macro
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

g4_color	macro
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lw	t2,4(a0)
	sll	t3,t3,25
	or	t2,t2,t3
	mtc2	t2,C2_RGB0
	endm

; Prepend one POLY_G4 to the ordering table, preserving the tag's high byte.
link_g4	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0800
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

g4_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,16(packet)
	swc2	C2_SXY2,24(packet)
	endm

clip_quad	macro vertex,area,skip
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	NCLIP
	lwc2	C2_VXY0,0(vertex)
	lwc2	C2_VZ0,4(vertex)
	mfc2	area,C2_MAC0
	nop
	blez	area,skip
	nop
	endm

quad_depth	macro skip
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	wAVSZ4
	endm

g4l_indices	macro
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

g4l_transform	macro reject
	load_vertex t2,C2_VXY0,C2_VZ0
	load_vertex t3,C2_VXY1,C2_VZ1
	load_vertex t4,C2_VXY2,C2_VZ2
	nop
	wRTPT
	lw	t5,20(a0)
	normal_addr t2
	normal_addr t3
	normal_addr t4
	srl	v1,t5,16
	sll	v1,v1,3
	addu	v1,v1,a1
	normal_addr t5
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,reject
	nop
	NCLIP
	lwc2	C2_VXY0,0(v1)
	lwc2	C2_VZ0,4(v1)
	mfc2	v1,C2_MAC0
	nop
	blez	v1,reject
	nop
	g4_xy a3
	nop
	wRTPS
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	lw	v0,4(a0)
	sll	t6,t6,25
	or	v0,v0,t6
	mtc2	v0,C2_RGB
	lwc2	C2_VXY1,0(t3)
	lwc2	C2_VZ1,4(t3)
	lwc2	C2_VXY2,0(t4)
	lwc2	C2_VZ2,4(t4)
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,reject
	nop
	wAVSZ4
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

; Shade vertex four separately before the triple command consumes the FIFO.
fourth_color	macro
	swc2	C2_RGB2,28(a3)
	lwc2	C2_VXY0,0(t2)
	lwc2	C2_VZ0,4(t2)
	endm

	section	.text

GsTMDfastNG4:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,ng4_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	ng4_vertices
ng4_loop:
	project3
	lhu	t5,26(a0)
	addu	v1,a0,zero
	addiu	a0,a0,28
	sll	t5,t5,3
	addu	t5,t5,a1
	ng4_vertices
	clip_quad t5,v0,ng4_next
	g4_xy a2
	nop
	wRTPS
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	lw	t5,4(v1)
	sll	t6,t6,25
	or	t5,t5,t6
	lw	t6,8(v1)
	lw	v0,12(v1)
	sw	t5,4(a2)
	sw	t6,12(a2)
	sw	v0,20(a2)
	quad_depth ng4_next
	lw	t5,16(v1)
	mfc2	t6,C2_OTZ
	sw	t5,28(a2)
	swc2	C2_SXY2,32(a2)
	link_g4 a2
	addiu	a2,a2,36
ng4_next:
	addiu	a3,a3,-1
	bne	a3,zero,ng4_loop
	nop
ng4_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastG4NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g4nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	g4nl_vertices
g4nl_loop:
	project3
	g4_color
	lhu	t5,22(a0)
	addiu	a0,a0,24
	sll	t5,t5,3
	addu	t5,t5,a1
	g4nl_vertices
	clip_quad t5,v0,g4nl_next
	g4_xy a3
	nop
	wRTPS
	quad_depth g4nl_next
	mfc2	t6,C2_OTZ
	nop
	link_g4 a3
	swc2	C2_RGB0,4(a3)
	swc2	C2_RGB0,12(a3)
	swc2	C2_RGB0,20(a3)
	swc2	C2_RGB0,28(a3)
	swc2	C2_SXY2,32(a3)
	addiu	a3,a3,36
g4nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,g4nl_loop
	nop
g4nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastG4L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g4l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
g4l_indices_next:
	g4l_indices
g4l_loop:
	g4l_transform g4l_skip
	wNCCS
	fourth_color
	wNCCT
	addiu	a0,a0,24
	link_g4 a3
	g4l_indices
	nop
	swc2	C2_SXY2,32(a3)
	swc2	C2_RGB0,4(a3)
	swc2	C2_RGB1,12(a3)
	swc2	C2_RGB2,20(a3)
	addiu	a3,a3,36
	addiu	t8,t8,-1
	bne	t8,zero,g4l_loop
	nop
	beq	zero,zero,g4l_return
	nop
g4l_skip:
	addiu	a0,a0,24
	addiu	t8,t8,-1
	bne	t8,zero,g4l_indices_next
	nop
g4l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastG4LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,g4lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
g4lfg_indices_next:
	g4l_indices
g4lfg_loop:
	g4l_transform g4lfg_skip
	wNCDS
	fourth_color
	wNCDT
	addiu	a0,a0,24
	link_g4 a3
	g4l_indices
	nop
	swc2	C2_SXY2,32(a3)
	swc2	C2_RGB0,4(a3)
	swc2	C2_RGB1,12(a3)
	swc2	C2_RGB2,20(a3)
	addiu	a3,a3,36
	addiu	t8,t8,-1
	bne	t8,zero,g4lfg_loop
	nop
	beq	zero,zero,g4lfg_return
	nop
g4lfg_skip:
	addiu	a0,a0,24
	addiu	t8,t8,-1
	bne	t8,zero,g4lfg_indices_next
	nop
g4lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
	nop
	nop
