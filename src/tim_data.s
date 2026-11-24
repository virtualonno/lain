; PlayStation TIM binary data objects in original link order.
	opt	c+
	section	.data

	xdef	gHorizonTim
	xdef	gHorizonTimFlags
gHorizonTim:
	include	"content/gHorizonTim.inc"
gHorizonTimFlags	equ	gHorizonTim+4

	xdef	gLifLogoTim
gLifLogoTim:
	include	"content/gLifLogoTim.inc"

	xdef	gLifeInstinctFunctionTim
gLifeInstinctFunctionTim:
	include	"content/gLifeInstinctFunctionTim.inc"

	xdef	gTextBandBarTim
gTextBandBarTim:
	include	"content/gTextBandBarTim.inc"

	xdef	gSiteALevelTim
gSiteALevelTim:
	include	"content/gSiteALevelTim.inc"

	xdef	gSiteBLevelTim
gSiteBLevelTim:
	include	"content/gSiteBLevelTim.inc"

	xdef	gRingTim
	xdef	gRingTimFlags
gRingTim:
	include	"content/gRingTim.inc"
gRingTimFlags	equ	gRingTim+4

	xdef	gLevelDigitsTim
	xdef	gLevelDigitsTimFlags
gLevelDigitsTim:
	include	"content/gLevelDigitsTim.inc"
gLevelDigitsTimFlags	equ	gLevelDigitsTim+4

	xdef	gEvenLevelTensTim
	xdef	gEvenLevelTensTimFlags
gEvenLevelTensTim:
	include	"content/gEvenLevelTensTim.inc"
gEvenLevelTensTimFlags	equ	gEvenLevelTensTim+4

	xdef	gEvenLevelOnesTim
	xdef	gEvenLevelOnesTimFlags
gEvenLevelOnesTim:
	include	"content/gEvenLevelOnesTim.inc"
gEvenLevelOnesTimFlags	equ	gEvenLevelOnesTim+4

	xdef	gOddLevelTensTim
	xdef	gOddLevelTensTimFlags
gOddLevelTensTim:
	include	"content/gOddLevelTensTim.inc"
gOddLevelTensTimFlags	equ	gOddLevelTensTim+4

	xdef	gOddLevelOnesTim
	xdef	gOddLevelOnesTimFlags
gOddLevelOnesTim:
	include	"content/gOddLevelOnesTim.inc"
gOddLevelOnesTimFlags	equ	gOddLevelOnesTim+4
