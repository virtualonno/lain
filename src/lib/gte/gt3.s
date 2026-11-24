	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastTNG3,GsTMDfastTG3NL,GsTMDfastTG3L,GsTMDfastTG3LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
tntg3_vertices	macro
	lw	t2,28(a0)
	lhu	t4,32(a0)
	srl	t3,t2,16
	sll	t2,t2,16
	srl	t2,t2,13
	sll	t3,t3,3
	sll	t4,t4,3
	addu	t2,t2,a1
	addu	t3,t3,a1
	addu	t4,t4,a1
	endm

tg3nl_vertices	macro
	lhu	t2,18(a0)
	lhu	t3,22(a0)
	lhu	t4,26(a0)
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

tng3_input_color	macro
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lb	t2,3(a0)
	sll	t3,t3,1
	or	t2,t2,t3
	lw	t3,16(a0)
	srl	t2,t2,1
	sll	t2,t2,25
	sll	t3,t3,8
	srl	t3,t3,8
	or	t2,t2,t3
	mtc2	t2,C2_RGB0
	lwc2	C2_RGB1,20(a0)
	lwc2	C2_RGB2,24(a0)
	endm

tg3_uv	macro packet
	lw	t2,4(a0)
	lw	t3,8(a0)
	lw	t4,12(a0)
	sw	t2,12(packet)
	sw	t3,24(packet)
	sw	t4,36(packet)
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

; Prepend one POLY_GT3 or POLY_FT3 to the ordering table, preserving the tag's high byte.
link_tg3	macro packet,length
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

tg3_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,20(packet)
	swc2	C2_SXY2,32(packet)
	endm

tg3_colors	macro packet
	swc2	C2_RGB0,4(packet)
	swc2	C2_RGB1,16(packet)
	swc2	C2_RGB2,28(packet)
	endm

tg3l_indices	macro
	lw	t2,16(a0)
	lw	t3,20(a0)
	lw	t4,24(a0)
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

tg3l_transform	macro skip
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
	normal_addr t2
	normal_addr t3
	normal_addr t4
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

GsTMDfastTNG3:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,tntg3_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tntg3_vertices
tntg3_loop:
	project3
	tng3_input_color
	tg3_uv a2
	addiu	a0,a0,36
	tntg3_vertices
	clip3 tntg3_next
	mfc2	t6,C2_OTZ
	tg3_xy a2
	tg3_colors a2
	link_tg3 a2,$0900
	addiu	a2,a2,40
tntg3_next:
	addiu	a3,a3,-1
	bne	a3,zero,tntg3_loop
	nop
tntg3_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastTG3NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg3nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	tg3nl_vertices
tg3nl_loop:
	project3
	; Without lighting this path emits a flat textured packet, not POLY_GT3.
	lwc2	C2_RGB0,4(a0)
	lwc2	C2_RGB1,8(a0)
	lwc2	C2_RGB2,12(a0)
	lb	t2,3(a0)
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	andi	t2,t2,2
	sll	t3,t3,1
	or	t2,t3,t2
	sll	t2,t2,24
	lui	v1,$2580
	ori	v1,v1,$8080
	or	v1,v1,t2
	sw	v1,4(a3)
	addiu	a0,a0,28
	tg3nl_vertices
	clip3 tg3nl_next
	mfc2	t6,C2_OTZ
	swc2	C2_RGB0,12(a3)
	swc2	C2_RGB1,20(a3)
	swc2	C2_RGB2,28(a3)
	swc2	C2_SXY0,8(a3)
	swc2	C2_SXY1,16(a3)
	swc2	C2_SXY2,24(a3)
	link_tg3 a3,$0700
	addiu	a3,a3,32
tg3nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,tg3nl_loop
	nop
tg3nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTG3L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg3l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
tg3l_indices_next:
	tg3l_indices
tg3l_loop:
	tg3l_transform tg3l_skip
	wNCCT
	link_tg3 a3,$0900
	tg3_uv a3
	tg3_xy a3
	tg3_colors a3
	addiu	a0,a0,28
	tg3l_indices
	addiu	a3,a3,40
	addiu	t8,t8,-1
	bne	t8,zero,tg3l_loop
	nop
	beq	zero,zero,tg3l_return
	nop
tg3l_skip:
	addiu	a0,a0,28
	addiu	t8,t8,-1
	bne	t8,zero,tg3l_indices_next
	nop
tg3l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastTG3LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,tg3lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
tg3lfg_indices_next:
	tg3l_indices
tg3lfg_loop:
	tg3l_transform tg3lfg_skip
	wNCDT
	link_tg3 a3,$0900
	tg3_uv a3
	tg3_xy a3
	tg3_colors a3
	addiu	a0,a0,28
	tg3l_indices
	addiu	a3,a3,40
	addiu	t8,t8,-1
	bne	t8,zero,tg3lfg_loop
	nop
	beq	zero,zero,tg3lfg_return
	nop
tg3lfg_skip:
	addiu	a0,a0,28
	addiu	t8,t8,-1
	bne	t8,zero,tg3lfg_indices_next
	nop
tg3lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
