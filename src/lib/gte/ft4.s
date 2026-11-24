	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastTNF4,GsTMDfastTF4NL,GsTMDfastTF4L,GsTMDfastTF4LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
tnf4_vertices	macro
	lw	t2,24(a0)
	lhu	t4,28(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

tf4nl_vertices	macro
	lw	t3,24(a0)
	lhu	t2,22(a0)
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
tf4_mode	macro
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lb	t2,3(a0)
	sll	t3,t3,1
	or	t2,t2,t3
	endm

tf4_neutral_color	macro base
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	lb	v1,3(base)
	sll	t6,t6,1
	or	v1,v1,t6
	sll	v1,v1,24
	lui	t6,$0080
	ori	t6,t6,$8080
	or	v1,v1,t6
	endm

; Resolve the fourth vertex, leaving the first three staged for the next loop.
advance_quad	macro
	addiu	a0,a0,32
	sll	v1,v1,3
	addu	v1,v1,a1
	endm

tf4_uv	macro packet
	addiu	v0,a0,-32
	lw	v1,4(v0)
	lw	t6,8(v0)
	sw	v1,12(packet)
	sw	t6,20(packet)
	lw	v1,12(v0)
	lw	t6,16(v0)
	sw	v1,28(packet)
	sw	t6,36(packet)
	endm

clip3	macro skip
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	NCLIP
	lwc2	C2_VXY0,0(v1)
	lwc2	C2_VZ0,4(v1)
	mfc2	v0,C2_MAC0
	nop
	blez	v0,skip
	nop
	endm

finish_quad	macro skip
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	wAVSZ4
	endm

; Prepend one POLY_FT4 to the ordering table, preserving the tag's high byte.
link_tf4	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0900
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

tf4_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,16(packet)
	swc2	C2_SXY2,24(packet)
	endm

tf4l_first_vertex	macro
	lw	t2,20(a0)
	lw	t3,24(a0)
	srl	t4,t2,16
	sll	t4,t4,3
	addu	t4,t4,a1
	endm

tf4l_transform	macro
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
	tf4_neutral_color a0
	mtc2	v1,C2_RGB
	sll	t5,t2,16
	srl	t5,t5,13
	addu	t5,t5,a2
	lhu	v1,28(a0)
	advance_quad
	tf4l_first_vertex
	endm

tf4l_normal_depth	macro
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

	section	.text

GsTMDfastTNF4:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,tnf4_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tnf4_vertices
tnf4_loop:
	project3
	lw	v0,20(a0)
	lhu	v1,30(a0)
	tf4_mode
	sll	v0,v0,8
	srl	v0,v0,8
	srl	t2,t2,1
	sll	t2,t2,25
	or	v0,v0,t2
	sw	v0,4(a2)
	advance_quad
	tnf4_vertices
	clip3 tnf4_next
	tf4_xy a2
	nop
	wRTPS
	tf4_uv a2
	finish_quad tnf4_next
	mfc2	t6,C2_OTZ
	swc2	C2_SXY2,32(a2)
	link_tf4 a2
	addiu	a2,a2,40
tnf4_next:
	addiu	a3,a3,-1
	bne	a3,zero,tnf4_loop
	nop
tnf4_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastTF4NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf4nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf4nl_vertices
tf4nl_loop:
	project3
	lhu	v1,28(a0)
	advance_quad
	tf4nl_vertices
	clip3 tf4nl_next
	tf4_xy a3
	nop
	wRTPS
	tf4_uv a3
	tf4_neutral_color v0
	sw	v1,4(a3)
	finish_quad tf4nl_next
	mfc2	t6,C2_OTZ
	swc2	C2_SXY2,32(a3)
	link_tf4 a3
	addiu	a3,a3,40
tf4nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf4nl_loop
	nop
tf4nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTF4L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf4l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf4l_first_vertex
tf4l_loop:
	tf4l_transform
	clip3 tf4l_next
	tf4_xy a3
	nop
	wRTPS
	tf4_uv a3
	finish_quad tf4l_next
	tf4l_normal_depth
	wNCCS
	link_tf4 a3
	swc2	C2_SXY2,32(a3)
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,40
tf4l_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf4l_loop
	nop
tf4l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTF4LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tf4lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tf4l_first_vertex
tf4lfg_loop:
	tf4l_transform
	clip3 tf4lfg_next
	tf4_xy a3
	nop
	wRTPS
	tf4_uv a3
	finish_quad tf4lfg_next
	tf4l_normal_depth
	wNCDS
	link_tf4 a3
	swc2	C2_SXY2,32(a3)
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,40
tf4lfg_next:
	addiu	t8,t8,-1
	bne	t8,zero,tf4lfg_loop
	nop
tf4lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
	nop
	nop
