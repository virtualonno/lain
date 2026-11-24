; NITR-BS frame decoder: unpacks variable-length coefficients into an MDEC command buffer.
	opt	c+,at-,m+

	xref	gChromaQuant
	xref	gLumaQuant
	xref	gSignedRunLevelCodes
	xdef	UnpackNitrBsFrame
	xdef	gNitrBsBitMasks
	xdef	gNitrBsRunLevelCodes

#define VLC_CODE_COUNT 111
#define VLC_SIGN_STRIDE (2*VLC_CODE_COUNT)
#define PADDED_COUNT_OFFSET 444
#define FRAME_COUNT_OFFSET 448
#define MDEC_HEADER_HI $3800
#define MDEC_EOB $fe00

; a0 points to the next input word. Assemble its stream bytes MSB first in s4.
finish_bit_word macro byte
	srl	t3,t3,8
	sll	s4,s4,8
	addu	byte,t3,zero
	andi	byte,byte,$ff
	or	s4,s4,byte
	srl	t3,t3,8
	sll	s4,s4,8
	addu	byte,t3,zero
	andi	byte,byte,$ff
	or	s4,s4,byte
	srl	t3,t3,8
	sll	s4,s4,8
	andi	t3,t3,$ff
	or	s4,s4,t3
	addiu	a0,a0,4
	endm

load_bit_word macro
	addu	s4,zero,zero
	lw	t8,0(a0)
	nop
	addu	t3,t8,zero
	andi	t8,t8,$ff
	or	s4,s4,t8
	finish_bit_word t8
	endm

; Emit the ten-bit DC level with the caller-selected quantizer, then its AC block.
decode_block macro qscale
	addu	t2,zero,zero
	addiu	t2,t2,10
	jal	.read_bits
	nop
	andi	t2,t2,$3ff
	or	t2,t2,qscale
	sh	t2,0(a1)
	addiu	a1,a1,2
	addiu	t1,t1,-1
	jal	.decode_ac_block
	nop
	endm

	section	.rdata
	dw	$5254494e
	dw	$2053422d
	dw	$31564552
	dw	$0000332e

	section	.text

; Unpack a NITR-BS frame into an MDEC command buffer.
; In: a0 = word-aligned NITR-BS payload (halfword count, then bitstream), a1 = MDEC command buffer.
; Uses: chroma and luma quantizers, run/level decode table.
UnpackNitrBsFrame:
	addiu	sp,sp,-40
	sw	a0,0(sp)
	sw	a1,4(sp)
	sw	s0,8(sp)
	sw	s1,12(sp)
	sw	s2,16(sp)
	sw	s3,20(sp)
	sw	s4,24(sp)
	sw	s5,28(sp)
	sw	s6,32(sp)
	sw	ra,36(sp)

	; Keep both sign tables and the halfword counts in scratchpad.
	addu	s0,zero,zero
	lui	s0,$1f80
	addu	t1,zero,zero
	addiu	t1,t1,VLC_CODE_COUNT
	addu	t2,s0,zero
	addu	t3,zero,zero
	la	t3,gSignedRunLevelCodes
	addu	t2,s0,zero
.copy_vlc_tables:
	lw	t4,0(t3)
	nop
	sw	t4,0(t2)
	addiu	t2,t2,4
	addiu	t3,t3,4
	addiu	t1,t1,-1
	bne	t1,zero,.copy_vlc_tables
	nop

	; Round the output length to a 64-halfword DMA block.
	lw	t1,0(a0)
	nop
	sw	t1,FRAME_COUNT_OFFSET(s0)
	addu	t2,t1,zero
	andi	t2,t2,$3f
	bne	t2,zero,.round_command_length
	nop
	sw	t1,PADDED_COUNT_OFFSET(s0)
	addu	t2,t1,zero
	j	.write_command_header
	nop
.round_command_length:
	addu	t3,t1,zero
	sub	t2,t3,t2
	addiu	t2,t2,64
	sw	t2,PADDED_COUNT_OFFSET(s0)
.write_command_header:
	addiu	a0,a0,4
	srl	t2,t2,1
	sh	t2,0(a1)
	addu	t2,zero,zero
	addiu	t2,t2,MDEC_HEADER_HI
	sh	t2,2(a1)
	addiu	a1,a1,4
	addu	t2,zero,zero
	la	t2,gChromaQuant
	addu	s1,zero,zero
	lhu	s1,0(t2)
	nop
	sll	s1,s1,10
	addu	t2,zero,zero
	la	t2,gLumaQuant
	addu	s2,zero,zero
	lhu	s2,0(t2)
	nop
	sll	s2,s2,10
	addu	s3,zero,zero
	ori	at,zero,MDEC_EOB
	addu	s3,s3,at
	addu	t0,zero,zero
	addu	s4,zero,zero
	load_bit_word
	addu	s5,zero,zero
	la	s5,gNitrBsBitMasks

; One macroblock has two chroma blocks and four luma blocks.
.decode_macroblock:
	decode_block s1
	decode_block s1
	addu	t6,zero,zero
	addiu	t6,t6,4
.decode_luma_block:
	decode_block s2
	addiu	t6,t6,-1
	bne	t6,zero,.decode_luma_block
	nop
	bgtz	t1,.decode_macroblock
	nop

	; Fill the rest of the rounded length with the MDEC padding code.
	addu	t1,zero,zero
	lw	t2,PADDED_COUNT_OFFSET(s0)
	lw	t3,FRAME_COUNT_OFFSET(s0)
	nop
	subu	t2,t2,t3
.pad_command:
	addu	t3,t2,zero
	subu	t3,t3,t1
	blez	t3,.restore_registers
	nop
	sh	s3,0(a1)
	addiu	a1,a1,2
	addiu	t1,t1,1
	j	.pad_command
	nop
.restore_registers:
	lw	a0,0(sp)
	lw	a1,4(sp)
	lw	s0,8(sp)
	lw	s1,12(sp)
	lw	s2,16(sp)
	lw	s3,20(sp)
	lw	s4,24(sp)
	lw	s5,28(sp)
	lw	s6,32(sp)
	lw	ra,36(sp)
	addiu	sp,sp,40
	jr	ra
	nop

; t2: requested width in, unsigned value out. t0 counts consumed bits in s4.
; a0 and s4 move forward together when the bit cursor crosses a word.
.read_bits:
	addu	t8,t2,zero
	addu	t3,zero,zero
	addiu	t3,t3,32
	sub	t3,t3,t0
	addu	t4,t3,zero
	subu	t3,t3,t2
	bltz	t3,.read_across_words
	nop
	addu	t4,t3,zero
	addu	t3,s4,zero
	srlv	t3,t3,t4
	addu	t4,s5,zero
	addu	t5,t2,zero
	addiu	t5,t5,-1
	sll	t5,t5,2
	add	t4,t4,t5
	lw	t5,0(t4)
	nop
	and	t2,t3,t5
	j	.advance_bit_cursor
	nop
.read_across_words:
	addu	t5,t4,zero
	addiu	t4,t4,-1
	sll	t4,t4,2
	addu	t3,s5,zero
	add	t3,t3,t4
	lw	t4,0(t3)
	addu	t2,s4,zero
	and	t2,t2,t4
	subu	t8,t8,t5
	addu	s4,zero,zero
	lw	t5,0(a0)
	nop
	addu	t3,t5,zero
	andi	t8,t8,$ff
	or	s4,s4,t5
	finish_bit_word t5
	beq	t8,zero,.resume_in_new_word
	nop
	sllv	t2,t2,t8
	addu	t5,zero,zero
	addiu	t5,t5,32
	subu	t5,t5,t8
	addu	t4,s4,zero
	srlv	t4,t4,t5
	addu	t5,t8,zero
	addiu	t5,t5,-1
	sll	t5,t5,2
	add	t5,s5,t5
	lw	t3,0(t5)
	nop
	and	t4,t4,t3
	or	t2,t2,t4
.resume_in_new_word:
	addu	t0,zero,zero
.advance_bit_cursor:
	add	t0,t0,t8
	addu	t3,t0,zero
	addiu	t3,t3,-32
	bne	t3,zero,.bits_ready
	nop
	addu	t0,zero,zero
	load_bit_word
.bits_ready:
	jr	ra
	nop

; Look ahead eight bits, then return the unused suffix for short codes.
; t9 preserves the block return address across the private bit-reader calls.
.decode_ac_block:
	addu	t9,ra,zero
.decode_ac:
	addu	t2,zero,zero
	addiu	t2,t2,8
	jal	.read_bits
	nop
	addiu	t1,t1,-1
	beq	t2,zero,.long_prefix
	nop
	addu	t3,t2,zero
	andi	t3,t3,$80
	beq	t3,zero,.leading_zero
	nop
	addu	t3,t2,zero
	andi	t3,t3,$40
	bne	t3,zero,.prefix_11
	nop
	sh	s3,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,6
	jal	.unread_bits
	nop
	j	.ac_block_done
	nop
.prefix_11:
	andi	t2,t2,$20
	bne	t2,zero,.prefix_11_negative
	nop
	lhu	t2,0(s0)
	j	.prefix_11_store
	nop
.prefix_11_negative:
	lhu	t2,VLC_SIGN_STRIDE(s0)
.prefix_11_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,5
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.leading_zero:
	addu	t3,t2,zero
	andi	t3,t3,$40
	beq	t3,zero,.two_leading_zeros
	nop
	addu	t3,t2,zero
	andi	t3,t3,$20
	beq	t3,zero,.prefix_010
	nop
	andi	t2,t2,$10
	bne	t2,zero,.prefix_011_negative
	nop
	lhu	t2,2(s0)
	j	.prefix_011_store
	nop
.prefix_011_negative:
	lhu	t2,VLC_SIGN_STRIDE+2(s0)
.prefix_011_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,4
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.prefix_010:
	addu	t3,t2,zero
	andi	t3,t3,$8
	srl	t2,t2,4
	andi	t2,t2,$3
	addiu	t2,t2,2
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_010_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_010_store
	nop
.prefix_010_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_010_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,3
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.two_leading_zeros:
	addu	t3,t2,zero
	andi	t3,t3,$20
	beq	t3,zero,.three_leading_zeros
	nop
	addu	t3,t2,zero
	andi	t3,t3,$18
	beq	t3,zero,.prefix_00100
	nop
	addu	t3,t2,zero
	andi	t3,t3,$4
	srl	t2,t2,3
	andi	t2,t2,$3
	addiu	t2,t2,3
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_001xx_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_001xx_store
	nop
.prefix_001xx_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_001xx_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,2
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.prefix_00100:
	addu	s6,t2,zero
	andi	s6,s6,$7
	addiu	s6,s6,15
	sll	s6,s6,1
	add	s6,s6,s0
	addu	t2,zero,zero
	addiu	t2,t2,1
	jal	.read_bits
	nop
	bne	t2,zero,.prefix_00100_negative
	nop
	lhu	t2,0(s6)
	j	.prefix_00100_store
	nop
.prefix_00100_negative:
	lhu	t2,VLC_SIGN_STRIDE(s6)
.prefix_00100_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.three_leading_zeros:
	addu	t3,t2,zero
	andi	t3,t3,$10
	beq	t3,zero,.four_leading_zeros
	nop
	addu	t3,t2,zero
	andi	t3,t3,$2
	srl	t2,t2,2
	andi	t2,t2,$3
	addiu	t2,t2,7
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_0001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_0001_store
	nop
.prefix_0001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_0001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,1
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.four_leading_zeros:
	addu	t3,t2,zero
	andi	t3,t3,$e
	beq	t3,zero,.seven_leading_zeros
	nop
	addu	t3,t2,zero
	andi	t3,t3,$8
	beq	t3,zero,.five_leading_zeros
	nop
	addu	t3,t2,zero
	andi	t3,t3,$1
	srl	t2,t2,1
	andi	t2,t2,$3
	addiu	t2,t2,11
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_00001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_00001_store
	nop
.prefix_00001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_00001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.five_leading_zeros:
	addu	t3,t2,zero
	andi	t3,t3,$4
	beq	t3,zero,.prefix_0000001
	nop
	; Escape: six run bits followed by a level byte and optional extension.
	andi	t2,t2,$3
	sll	t2,t2,12
	addu	s6,t2,zero
	addu	t2,zero,zero
	addiu	t2,t2,12
	jal	.read_bits
	nop
	or	t2,t2,s6
	addu	s6,t2,zero
	andi	t2,t2,$ff
	srl	s6,s6,8
	bne	t2,zero,.escape_nonzero_level
	nop
	addiu	t2,t2,8
	jal	.read_bits
	nop
	j	.escape_pack
	nop
.escape_nonzero_level:
	addu	t3,t2,zero
	addiu	t3,t3,-128
	bne	t3,zero,.escape_level_byte
	nop
	addu	t2,zero,zero
	addiu	t2,t2,8
	jal	.read_bits
	nop
	addu	t3,zero,zero
	addiu	t3,t3,256
	subu	t3,t3,t2
	addu	t2,zero,zero
	subu	t2,t2,t3
	j	.escape_pack
	nop
.escape_level_byte:
	addu	t3,t2,zero
	andi	t3,t3,$80
	beq	t3,zero,.escape_pack
	nop
	addu	t3,zero,zero
	lui	t3,$ffff
	ori	t3,t3,$ff00
	or	t2,t2,t3
.escape_pack:
	andi	t2,t2,$3ff
	sll	s6,s6,10
	or	t2,t2,s6
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.prefix_0000001:
	addu	s6,t2,zero
	andi	s6,s6,$1
	sll	s6,s6,3
	addu	t2,zero,zero
	addiu	t2,t2,3
	jal	.read_bits
	nop
	or	t2,t2,s6
	addu	t3,t2,zero
	andi	t3,t3,$1
	srl	t2,t2,1
	addiu	t2,t2,23
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_0000001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_0000001_store
	nop
.prefix_0000001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_0000001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.seven_leading_zeros:
	addu	t2,zero,zero
	addiu	t2,t2,5
	jal	.read_bits
	nop
	addu	t3,t2,zero
	andi	t3,t3,$1
	srl	t2,t2,1
	addiu	t2,t2,31
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_00000001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_00000001_store
	nop
.prefix_00000001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_00000001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.long_prefix:
	addu	t2,zero,zero
	addiu	t2,t2,8
	jal	.read_bits
	nop
	addu	t3,t2,zero
	andi	t3,t3,$80
	beq	t3,zero,.prefix_000000000
	nop
	addu	t3,t2,zero
	andi	t3,t3,$4
	srl	t2,t2,3
	andi	t2,t2,$f
	addiu	t2,t2,47
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_000000001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_000000001_store
	nop
.prefix_000000001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_000000001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,2
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.prefix_000000000:
	addu	t3,t2,zero
	andi	t3,t3,$40
	beq	t3,zero,.prefix_0000000000
	nop
	addu	t3,t2,zero
	andi	t3,t3,$2
	srl	t2,t2,2
	andi	t2,t2,$f
	addiu	t2,t2,63
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_0000000001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_0000000001_store
	nop
.prefix_0000000001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_0000000001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	addu	t2,zero,zero
	addiu	t2,t2,1
	jal	.unread_bits
	nop
	j	.decode_ac
	nop
.prefix_0000000000:
	addu	t3,t2,zero
	andi	t3,t3,$20
	beq	t3,zero,.prefix_00000000000
	nop
	addu	t3,t2,zero
	andi	t3,t3,$1
	srl	t2,t2,1
	andi	t2,t2,$f
	addiu	t2,t2,79
	sll	t2,t2,1
	add	t2,t2,s0
	bne	t3,zero,.prefix_00000000001_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_00000000001_store
	nop
.prefix_00000000001_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_00000000001_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.prefix_00000000000:
	addu	s6,t2,zero
	andi	s6,s6,$f
	addu	t2,zero,zero
	addiu	t2,t2,1
	jal	.read_bits
	nop
	addu	t3,t2,zero
	addiu	s6,s6,95
	sll	s6,s6,1
	add	t2,s6,s0
	bne	t3,zero,.prefix_00000000000_negative
	nop
	lhu	t2,0(t2)
	j	.prefix_00000000000_store
	nop
.prefix_00000000000_negative:
	lhu	t2,VLC_SIGN_STRIDE(t2)
.prefix_00000000000_store:
	nop
	sh	t2,0(a1)
	addiu	a1,a1,2
	j	.decode_ac
	nop
.ac_block_done:
	addu	ra,t9,zero
	jr	ra
	nop

; Return t2 unused lookahead bits, reloading the previous word if needed.
.unread_bits:
	subu	t0,t0,t2
	bgez	t0,.rewind_done
	nop
	addiu	t0,t0,32
	addiu	a0,a0,-8
	load_bit_word
.rewind_done:
	jr	ra
	nop

	section	.data
; Bit masks indexed by read width minus one.
gNitrBsBitMasks:
	dw	$00000001
	dw	$00000003
	dw	$00000007
	dw	$0000000f
	dw	$0000001f
	dw	$0000003f
	dw	$0000007f
	dw	$000000ff
	dw	$000001ff
	dw	$000003ff
	dw	$000007ff
	dw	$00000fff
	dw	$00001fff
	dw	$00003fff
	dw	$00007fff
	dw	$0000ffff
	dw	$0001ffff
	dw	$0003ffff
	dw	$0007ffff
	dw	$000fffff
	dw	$001fffff
	dw	$003fffff
	dw	$007fffff
	dw	$00ffffff
	dw	$01ffffff
	dw	$03ffffff
	dw	$07ffffff
	dw	$0fffffff
	dw	$1fffffff
	dw	$3fffffff
	dw	$7fffffff
	dw	$ffffffff
; MDEC run/level codes (run in the top 6 bits, level in the low 10); table init reads the first 111.
gNitrBsRunLevelCodes:
	dh	$0001,$0002,$0401,$0003,$0004,$0801,$0005,$0006
	dh	$0c01,$0402,$0007,$0008,$1001,$0009,$1401,$000a
	dh	$000b,$0403,$1801,$000c,$000d,$1c01,$000e,$000f
	dh	$0802,$2001,$0404,$0010,$0011,$2401,$0012,$0013
	dh	$0405,$0014,$2801,$0015,$0c02,$3001,$0017,$0016
	dh	$2c01,$0018,$001c,$0019,$0406,$0803,$001b,$001a
	dh	$3401,$001d,$0407,$1002,$001f,$001e,$3801,$0020
	dh	$0021,$0408,$0023,$0022,$1402,$0024,$0025,$0804
	dh	$0409,$0418,$0026,$3c01,$0027,$0c03,$1c03,$0028
	dh	$0029,$002a,$002b,$040a,$002c,$1802,$002d,$002f
	dh	$002e,$4001,$0805,$0030,$040b,$0031,$0033,$0032
	dh	$1c02,$0034,$1003,$0035,$4401,$040c,$0037,$0036
	dh	$0038,$0039,$5401,$003a,$0c04,$040d,$5c01,$2002
	dh	$003b,$0806,$4c01,$003c,$2402,$6001,$4801,$003d
	dh	$040e,$5001,$003e,$0040,$003f,$1403,$0041,$6401
	dh	$0042,$5801,$0043,$040f,$3402,$0044,$0045,$0807
	dh	$0046,$0047,$1004,$0410,$0048,$0c05,$0049,$0411
	dh	$004a,$1803,$004b,$3403,$2c02,$0808,$0413,$004c
	dh	$2802,$6801,$004d,$004e,$0412,$004f,$0c06,$0417
	dh	$0050,$0051,$1404,$0053,$0052,$2403,$0809,$0054
	dh	$3002,$0414,$0055,$0056,$7801,$0057,$1005,$0058
	dh	$2003,$0415,$0059,$1c04,$005a,$0419,$7001,$080a
	dh	$005b,$005c,$6c01,$0416,$0c07,$0060,$1804,$0061
	dh	$005e,$005d,$2404,$0063,$005f,$0064,$0062,$0065
	dh	$080b,$1405,$7401,$0066,$0067,$041a,$0c08,$0068
	dh	$3404,$0069,$006a,$2405,$041b,$3414,$006b,$1006
	dh	$2004,$080c,$2406,$041c,$006c,$7c01,$2803,$3415
	dh	$2408,$006d,$3802,$006e,$041d,$080d,$006f,$1805
	dh	$041e,$0070,$2c03,$0c09,$8001,$1007,$0071,$3003
	dh	$1c05,$1406,$3405,$0072,$080e,$1806,$0074,$041f
	dh	$0077,$2409,$240a,$0073,$0075,$2407,$2005,$0076
	dh	$007a,$0c0e,$0078,$3406,$007b,$0c0a,$080f
