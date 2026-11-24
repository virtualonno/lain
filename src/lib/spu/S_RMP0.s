; LIBSPU S_RMP0: reverb work-area bases in 8-byte SPU RAM units.
	opt	c+,at-,m-

	xdef	_spu_rev_startaddr

	section	.data
_spu_rev_startaddr:
	dw	$fffe	; SPU_REV_MODE_OFF
	dw	$fb28	; SPU_REV_MODE_ROOM
	dw	$fc18	; SPU_REV_MODE_STUDIO_A
	dw	$f6f8	; SPU_REV_MODE_STUDIO_B
	dw	$f204	; SPU_REV_MODE_STUDIO_C
	dw	$ea44	; SPU_REV_MODE_HALL
	dw	$e128	; SPU_REV_MODE_SPACE
	dw	$cff8	; SPU_REV_MODE_ECHO
	dw	$cff8	; SPU_REV_MODE_DELAY
	dw	$f880	; SPU_REV_MODE_PIPE
	dw	0,0	; Trailing zeros outside SPU_REV_MODE_MAX.
