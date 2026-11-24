	opt	c+

SWAP_CACHES	equ	$00020000	; the status register's cache swap bit
EOB		equ	$fe00		; the code that ends a block
ESCAPE		equ	$7c1f		; the stream's next 16 bits are the code

	xref	CF2_DVLC
	xref	CF_DVLC
	xdef	DecDCTvlcSize
	xdef	DecDCTvlc

	section	.text
; the output limit DecDCTvlcSize sets, in halfwords
vlc_size:
	dw	$00ffffff

DecDCTvlcSize:
	la	t0,vlc_size
	addi	at,a0,-1
	blez	at,.Lunlimited
	lw	v0,0(t0)
	sll	at,a0,1
	jr	ra
	sw	at,0(t0)
.Lunlimited:
	li	at,$ffffff
	jr	ra
	sw	at,0(t0)

DecDCTvlc:
	la	t0,vlc_size
	la	a2,CF_DVLC
	la	a3,CF2_DVLC
	bne	a0,zero,.Lnew_stream
	lw	t1,0(t0)
	la	t0,vlc_state
	lw	a0,0(t0)
	lw	a1,4(t0)
	lw	v0,8(t0)
	lw	v1,12(t0)
	lw	t4,16(t0)
	lw	t5,20(t0)
	lw	t7,24(t0)
	lw	t8,28(t0)
	lw	t9,32(t0)
	add	t1,t1,t1
	bgez	zero,.Lac
	add	t6,a1,t1
.Lnew_stream:
	add	t5,zero,zero
	add	t7,zero,zero
	add	t8,zero,zero
	add	t9,zero,zero
	add	t1,t1,t1
	add	t6,a1,t1
	lhu	t0,0(a0)
	lhu	t1,2(a0)
	lhu	t4,4(a0)
	lhu	t2,6(a0)
	lhu	v0,8(a0)
	lhu	v1,10(a0)
	addi	t2,t2,-3
	bltz	t2,.Lheader_done
	sll	t4,t4,10
	addi	t5,zero,1
.Lheader_done:
	addi	a0,a0,12
	sll	v0,v0,16
	or	v0,v0,v1
	or	v1,zero,zero
	sh	t0,0(a1)
	sh	t1,2(a1)
	addi	a1,a1,2
.Lblock:
	beq	t5,zero,.Ldc_fixed
	srl	t0,v0,22
	xori	at,t0,$3ff
	beq	at,zero,.Lend_of_data
	addi	a1,a1,2
	addi	at,t5,-3
	bltz	at,.Ldc_lookup
	addi	at,a2,-1024
	addi	at,at,-1024
.Ldc_lookup:
	srl	t0,v0,24
	sll	t0,t0,2
	add	t0,t0,at
	lhu	t1,0(t0)
	lhu	t2,2(t0)
	and	t0,zero,zero
	beq	t2,zero,.Ldc_consume
	sllv	v0,v0,t1
	addi	at,zero,32
	sub	at,at,t2
	srlv	t0,v0,at
	bltz	v0,.Ldc_signed
	sllv	v0,v0,t2
	addi	t3,zero,-1
	srlv	t3,t3,at
	sub	t0,t0,t3
.Ldc_signed:
	add	v1,v1,t2
.Ldc_consume:
	add	v1,v1,t1
	andi	at,v1,$10
	beq	at,zero,.Ldc_predict
	andi	v1,v1,$f
	lhu	t1,0(a0)
	addi	a0,a0,2
	sllv	t1,t1,v1
	or	v0,v0,t1
.Ldc_predict:
	addi	at,t5,-2
	bgtz	at,.Ldc_y
	add	t1,t9,t0
	beq	at,zero,.Ldc_cb
	add	t1,t8,t0
	add	t1,t7,t0
	bgez	zero,.Ldc_store
	add	t7,t7,t0
.Ldc_cb:
	bgez	zero,.Ldc_store
	add	t8,t8,t0
.Ldc_y:
	add	t9,t9,t0
.Ldc_store:
	sll	t1,t1,2
	andi	t1,t1,$3ff
	or	t1,t4,t1
	addi	t5,t5,1
	addi	at,t5,-7
	bne	at,zero,.Lcheck_size
	sh	t1,0(a1)
	bgez	zero,.Lcheck_size
	addi	t5,t5,-6
.Ldc_fixed:
	xori	at,t0,$1ff
	beq	at,zero,.Lend_of_data
	addi	a1,a1,2
	sll	v0,v0,10
	addi	v1,v1,10
	andi	at,v1,$10
	beq	at,zero,.Ldc_fixed_store
	andi	v1,v1,$f
	lhu	t1,0(a0)
	addi	a0,a0,2
	sllv	t1,t1,v1
	or	v0,v0,t1
.Ldc_fixed_store:
	or	t0,t4,t0
	sh	t0,0(a1)
.Lcheck_size:
	subu	at,a1,t6
	bgez	at,.Lsuspend
	addi	a1,a1,2
.Lac:
	srl	t0,v0,19
	sll	t0,t0,3
	add	t0,t0,a2
	lw	t1,0(t0)
	nop
	bne	t1,zero,.Lac_pair
	andi	at,t1,$ff
	sll	v0,v0,8
	addi	v1,v1,8
	andi	at,v1,$10
	beq	at,zero,.Lac_long
	andi	v1,v1,$f
	lhu	t0,0(a0)
	addi	a0,a0,2
	sllv	t0,t0,v1
	or	v0,v0,t0
.Lac_long:
	srl	t0,v0,23
	sll	t0,t0,2
	add	t0,t0,a3
	lw	t1,0(t0)
	add	t3,zero,zero
	bgez	zero,.Lac_consume
	andi	at,t1,$ff
.Lac_pair:
	lw	t3,4(t0)
.Lac_consume:
	sllv	v0,v0,at
	add	v1,v1,at
	andi	at,v1,$10
	beq	at,zero,.Lac_store
	andi	v1,v1,$f
	lhu	t0,0(a0)
	addi	a0,a0,2
	sllv	t0,t0,v1
	or	v0,v0,t0
.Lac_store:
	srl	t1,t1,16
	xori	at,t1,ESCAPE
	beq	at,zero,.Lescape
	xori	at,t1,EOB
	beq	at,zero,.Lblock
	sh	t1,0(a1)
	beq	t3,zero,.Lac
	addi	a1,a1,2
	andi	t2,t3,$ffff
	xori	at,t2,ESCAPE
	beq	at,zero,.Lescape
	xori	at,t2,EOB
	beq	at,zero,.Lblock
	sh	t2,0(a1)
	srl	t2,t3,16
	beq	t2,zero,.Lac
	addi	a1,a1,2
	xori	at,t2,ESCAPE
	beq	at,zero,.Lescape
	xori	at,t2,EOB
	beq	at,zero,.Lblock
	sh	t2,0(a1)
	bgez	zero,.Lac
	addi	a1,a1,2
.Lescape:
	srl	t0,v0,16
	sh	t0,0(a1)
	addi	a1,a1,2
	lhu	t0,0(a0)
	addi	a0,a0,2
	sll	v0,v0,16
	sllv	t0,t0,v1
	bgez	zero,.Lac
	or	v0,v0,t0
.Lend_of_data:
	ori	t0,zero,EOB
	addi	v0,zero,64
.Lfill_end:
	sh	t0,0(a1)
	addi	a1,a1,2
	bne	v0,zero,.Lfill_end
	addi	v0,v0,-1
	mfc0	t1,status
	nop
	lui	at,SWAP_CACHES>>16
	or	t1,t1,at
	mtc0	t1,status
	jr	ra
	add	v0,zero,zero
.Lsuspend:
	la	t0,vlc_state
	sw	a0,0(t0)
	sw	a1,4(t0)
	sw	v0,8(t0)
	sw	v1,12(t0)
	sw	t4,16(t0)
	sw	t5,20(t0)
	sw	t7,24(t0)
	sw	t8,28(t0)
	sw	t9,32(t0)
	jr	ra
	addi	v0,zero,1
; where a call that filled its buffer stopped: bs, buf, the bit buffer and its count, the
; quantizer scale, the block, the three DC predictors
vlc_state:
	dsw	9
