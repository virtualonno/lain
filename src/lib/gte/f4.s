	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastNF4,GsTMDfastF4NL,GsTMDfastF4L,GsTMDfastF4LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
nf4_vertices	macro
	lw	t2,8(a0)
	lhu	t4,12(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

f4nl_vertices	macro
	lw	t3,12(a0)
	lhu	t2,10(a0)
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

f4_color	macro dest
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lw	t2,4(a0)
	sll	t3,t3,25
	or	t2,t2,t3
	mtc2	t2,dest
	endm

advance_quad	macro stride
	addiu	a0,a0,stride
	sll	v1,v1,3
	addu	v1,v1,a1
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

project_fourth	macro skip
	nop
	wRTPS
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	wAVSZ4
	endm

; Prepend one POLY_F4 to the ordering table, preserving the tag's high byte.
link_f4	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0500
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

f4_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,12(packet)
	swc2	C2_SXY2,16(packet)
	endm

f4l_first_vertex	macro
	lw	t2,8(a0)
	lw	t3,12(a0)
	srl	t4,t2,16
	sll	t4,t4,3
	addu	t4,t4,a1
	endm

f4l_transform	macro
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
	lhu	v1,16(a0)
	f4_color C2_RGB
	advance_quad 20
	f4l_first_vertex
	endm

f4l_normal_depth	macro
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

	section	.text

GsTMDfastNF4:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,nf4_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	nf4_vertices
nf4_loop:
	project3
	f4_color C2_RGB2
	lhu	v1,14(a0)
	advance_quad 16
	nf4_vertices
	clip3 nf4_next
	swc2	C2_RGB2,4(a2)
	f4_xy a2
	project_fourth nf4_next
	mfc2	t6,C2_OTZ
	swc2	C2_SXY2,20(a2)
	link_f4 a2
	addiu	a2,a2,24
nf4_next:
	addiu	a3,a3,-1
	bne	a3,zero,nf4_loop
	nop
nf4_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastF4NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f4nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f4nl_vertices
f4nl_loop:
	project3
	f4_color C2_RGB2
	lhu	v1,16(a0)
	advance_quad 20
	f4nl_vertices
	clip3 f4nl_next
	swc2	C2_RGB2,4(a3)
	f4_xy a3
	project_fourth f4nl_next
	mfc2	t6,C2_OTZ
	swc2	C2_SXY2,20(a3)
	link_f4 a3
	addiu	a3,a3,24
f4nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,f4nl_loop
	nop
f4nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastF4L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f4l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f4l_first_vertex
f4l_loop:
	f4l_transform
	clip3 f4l_next
	f4_xy a3
	project_fourth f4l_next
	f4l_normal_depth
	wNCCS
	link_f4 a3
	swc2	C2_SXY2,20(a3)
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,24
f4l_next:
	addiu	t8,t8,-1
	bne	t8,zero,f4l_loop
	nop
f4l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastF4LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f4lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f4l_first_vertex
f4lfg_loop:
	f4l_transform
	clip3 f4lfg_next
	f4_xy a3
	project_fourth f4lfg_next
	f4l_normal_depth
	wNCDS
	link_f4 a3
	swc2	C2_SXY2,20(a3)
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,24
f4lfg_next:
	addiu	t8,t8,-1
	bne	t8,zero,f4lfg_loop
	nop
f4lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
	nop
	nop
