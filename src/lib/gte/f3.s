	opt	c+,at-,m-
#include <gtereg.h>
#include <hilo.h>
	include	"gte_word.inc"

	xdef	GsTMDfastNF3,GsTMDfastF3NL,GsTMDfastF3L,GsTMDfastF3LFG
	xref	GsTON

; Resolve the packed vertex indices into SVECTOR addresses before RTPT.
nf3_vertices	macro
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

f3nl_vertices	macro
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

f3_color	macro dest
	lui	t3,HI(GsTON)
	lw	t3,LO(GsTON)(t3)
	lw	t2,4(a0)
	sll	t3,t3,25
	or	t2,t2,t3
	mtc2	t2,dest
	addiu	a0,a0,16
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

; Prepend one POLY_F3 to the ordering table, preserving the tag's high byte.
link_f3	macro packet
	subu	t6,t6,t1
	srlv	t6,t6,t7
	andi	t6,t6,$ffff
	sll	t6,t6,2
	addu	t6,t6,t0
	lw	t5,0(t6)
	lui	v0,$0400
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

f3_xy	macro packet
	swc2	C2_SXY0,8(packet)
	swc2	C2_SXY1,12(packet)
	swc2	C2_SXY2,16(packet)
	endm

f3l_first_vertex	macro
	lw	t2,8(a0)
	lw	t3,12(a0)
	srl	t4,t2,16
	sll	t4,t4,3
	addu	t4,t4,a1
	endm

f3l_transform	macro
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
	f3_color C2_RGB
	f3l_first_vertex
	endm

f3l_normal_depth	macro
	lwc2	C2_VXY0,0(t5)
	lwc2	C2_VZ0,4(t5)
	mfc2	t6,C2_OTZ
	nop
	endm

	section	.text

GsTMDfastNF3:
	lw	t9,20(sp)
	lw	t7,16(sp)
	beq	a3,zero,nf3_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	nf3_vertices
nf3_loop:
	project3
	f3_color C2_RGB2
	nf3_vertices
	clip3 nf3_next
	mfc2	t6,C2_OTZ
	nop
	link_f3 a2
	swc2	C2_RGB2,4(a2)
	f3_xy a2
	addiu	a2,a2,20
nf3_next:
	addiu	a3,a3,-1
	bne	a3,zero,nf3_loop
	nop
nf3_return:
	addu	v0,a2,zero
	jr	ra
	nop

GsTMDfastF3NL:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f3nl_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f3nl_vertices
f3nl_loop:
	project3
	f3_color C2_RGB2
	f3nl_vertices
	clip3 f3nl_next
	mfc2	t6,C2_OTZ
	nop
	link_f3 a3
	swc2	C2_RGB2,4(a3)
	f3_xy a3
	addiu	a3,a3,20
f3nl_next:
	addiu	t8,t8,-1
	bne	t8,zero,f3nl_loop
	nop
f3nl_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastF3L:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f3l_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f3l_first_vertex
f3l_loop:
	f3l_transform
	clip3 f3l_next
	f3l_normal_depth
	wNCCS
	link_f3 a3
	f3_xy a3
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,20
f3l_next:
	addiu	t8,t8,-1
	bne	t8,zero,f3l_loop
	nop
f3l_return:
	addu	v0,a3,zero
	jr	ra
	nop

GsTMDfastF3LFG:
	lw	t8,16(sp)
	lw	t9,24(sp)
	lw	t7,20(sp)
	beq	t8,zero,f3lfg_return
	lw	t0,4(t9)
	lw	t1,8(t9)
	f3l_first_vertex
f3lfg_loop:
	f3l_transform
	clip3 f3lfg_next
	f3l_normal_depth
	wNCDS
	link_f3 a3
	f3_xy a3
	swc2	C2_RGB2,4(a3)
	addiu	a3,a3,20
f3lfg_next:
	addiu	t8,t8,-1
	bne	t8,zero,f3lfg_loop
	nop
f3lfg_return:
	addu	v0,a3,zero
	jr	ra
	nop
	nop
