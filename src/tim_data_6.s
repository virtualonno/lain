; PlayStation TIM binary data objects in original link order.
	opt	c+
	section	.data

	xdef	gGateSlabTim
	xdef	gGateSlabTimFlags
gGateSlabTim:
	include	"content/gGateSlabTim.inc"
gGateSlabTimFlags	equ	gGateSlabTim+4

	xdef	gGateBackdropTim
	xdef	gGateBackdropTimFlags
gGateBackdropTim:
	include	"content/gGateBackdropTim.inc"
gGateBackdropTimFlags	equ	gGateBackdropTim+4
