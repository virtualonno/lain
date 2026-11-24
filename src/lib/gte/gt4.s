	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastTNG4,GsTMDfastTG4NL,GsTMDfastTG4L,GsTMDfastTG4LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
tng4_vertices	macro
	lw	t2,36(a0)
	lhu	t4,40(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

tg4nl_vertices	macro
	lhu	t2,22(a0)
	lhu	t3,26(a0)
	lhu	t4,30(a0)
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

tng4_input_color	macro
	lui	v1,HI(GsTON)
	lw	v1,LO(GsTON)(v1)
	lb	v0,3(a0)
	sll	v1,v1,1
	or	v0,v0,v1
	lw	v1,20(a0)
	srl	v0,v0,1
	sll	v0,v0,25
	sll	v1,v1,8
	srl	v1,v1,8
	or	v0,v0,v1
	mtc2	v0,C2_RGB0
	lwc2	C2_RGB1,24(a0)
	lwc2	C2_RGB2,28(a0)
	lwc2	C2_RGB,32(a0)
	endm

start_quad_uv	macro
	lw	t5,4(v1)
	lw	t6,8(v1)
	endm

finish_quad_uv	macro packet,uv1,uv2,uv3,lastxy
	lw	v0,12(v1)
	sw	t5,12(packet)
	sw	t6,uv1(packet)
	sw	v0,uv2(packet)
	lw	t5,16(v1)
	mfc2	t6,C2_OTZ
	sw	t5,uv3(packet)
	swc2	C2_SXY2,lastxy(packet)
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

; Prepend one POLY_GT4 or POLY_FT4 to the ordering table, preserving the tag's high byte.
link_tg4	macro packet,length
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,length
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

tg4_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,20(packet)
	swc2	C2_SXY2,32(packet)
	endm

tg4_colors	macro packet
	swc2	C2_RGB0,4(packet)
	swc2	C2_RGB1,16(packet)
	swc2	C2_RGB2,28(packet)
	endm

tg4l_indices	macro
	lw	t2,20(a0)
	lw	t3,24(a0)
	lw	t4,28(a0)
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

tg4l_transform	macro skip
	load_vertex t2,C2_VXY0,C2_VZ0
	load_vertex t3,C2_VXY1,C2_VZ1
	load_vertex t4,C2_VXY2,C2_VZ2
	nop
	wRTPT
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	lb	v0,3(a0)
	sll	t6,t6,1
	or	v0,v0,t6
	sll	v0,v0,24
	lui	t6,$0080
	ori	t6,t6,$8080
	or	v0,v0,t6
	mtc2	v0,C2_RGB
	lw	t5,32(a0)
	normal_addr t2
	normal_addr t3
	normal_addr t4
	srl	v1,t5,16
	sll	v1,v1,3
	addu	v1,v1,a1
	normal_addr t5
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	NCLIP
	lwc2	C2_VXY0,0(v1)
	lwc2	C2_VZ0,4(v1)
	mfc2	v1,C2_MAC0
	nop
	blez	v1,skip
	nop
	tg4_xy a3
	nop
	wRTPS
	lwc2	C2_VXY1,0(t3)
	lwc2	C2_VZ1,4(t3)
	lwc2	C2_VXY2,0(t4)
	lwc2	C2_VZ2,4(t4)
	cfc2	v0,C2_FLAG
	nop
	bltz	v0,skip
	nop
	wAVSZ4
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

; Store the UV words while the fourth lighting result is becoming ready.
tg4l_uv_and_fourth_color	macro
	lw	v0,4(a0)
	lw	t3,8(a0)
	lw	t4,12(a0)
	lw	t5,16(a0)
	sw	v0,12(a3)
	sw	t3,24(a3)
	sw	t4,36(a3)
	sw	t5,48(a3)
	swc2	C2_RGB2,40(a3)
	lwc2	C2_VXY0,0(t2)
	lwc2	C2_VZ0,4(t2)
	endm

	section	.text

GsTMDfastTNG4:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,tng4_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tng4_vertices
tng4_loop:
	project3
	tng4_input_color
	lhu	t5,42(a0)
	addu	v1,a0,zero
	addiu	a0,a0,44
	sll	t5,t5,3
	addu	t5,t5,a1
	tng4_vertices
	clip_quad t5,v0,tng4_next
	tg4_xy a2
	nop
	wRTPS
	start_quad_uv
	quad_depth tng4_next
	finish_quad_uv a2,24,36,48,44
	tg4_colors a2
	swc2	C2_RGB,40(a2)
	link_tg4 a2,$0c00
	addiu	a2,a2,52
tng4_next:
	addiu	a3,a3,-1
	bne	a3,zero,tng4_loop
	nop
tng4_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastTG4NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg4nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tg4nl_vertices
tg4nl_loop:
	project3
	lhu	t5,34(a0)
	addu	v1,a0,zero
	addiu	a0,a0,36
	sll	t5,t5,3
	addu	t5,t5,a1
	tg4nl_vertices
	clip_quad t5,v0,tg4nl_next
	; Without lighting the output is POLY_FT4.
	swc2	C2_SXY0,8(a3)
	swc2	C2_SXY1,16(a3)
	swc2	C2_SXY2,24(a3)
	nop
	wRTPS
	lb	t5,3(v1)
	lui	t6,HI(GsTON)
	lw	t6,LO(GsTON)(t6)
	andi	t5,t5,2
	sll	t6,t6,1
	or	t5,t6,t5
	sll	t5,t5,24
	lui	t6,$2d80
	ori	t6,t6,$8080
	or	t6,t5,t6
	sw	t6,4(a3)
	start_quad_uv
	quad_depth tg4nl_next
	finish_quad_uv a3,20,28,36,32
	link_tg4 a3,$0900
	addiu	a3,a3,40
tg4nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,tg4nl_loop
	nop
tg4nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTG4L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg4l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
tg4l_indices_next:
	tg4l_indices
tg4l_loop:
	tg4l_transform tg4l_skip
	wNCCS
	tg4l_uv_and_fourth_color
	wNCCT
	link_tg4 a3,$0c00
	addiu	a0,a0,36
	tg4l_indices
	swc2	C2_SXY2,44(a3)
	tg4_colors a3
	addiu	a3,a3,52
	addiu	t8,t8,-1
	bne	t8,zero,tg4l_loop
	nop
	beq	zero,zero,tg4l_return
	nop
tg4l_skip:
	addiu	a0,a0,36
	addiu	t8,t8,-1
	bne	t8,zero,tg4l_indices_next
	nop
tg4l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTG4LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg4lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
tg4lfg_indices_next:
	tg4l_indices
tg4lfg_loop:
	tg4l_transform tg4lfg_skip
	wNCDS
	tg4l_uv_and_fourth_color
	wNCDT
	addiu	a0,a0,36
	link_tg4 a3,$0c00
	tg4l_indices
	swc2	C2_SXY2,44(a3)
	tg4_colors a3
	addiu	a3,a3,52
	addiu	t8,t8,-1
	bne	t8,zero,tg4lfg_loop
	nop
	beq	zero,zero,tg4lfg_return
	nop
tg4lfg_skip:
	addiu	a0,a0,36
	addiu	t8,t8,-1
	bne	t8,zero,tg4lfg_indices_next
	nop
tg4lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
