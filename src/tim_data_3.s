; PlayStation TIM binary data objects in original link order.
	opt	c+
	section	.data

	xdef	gQuadTim
	xdef	gQuadTimFlags
gQuadTim:
	include	"content/gQuadTim.inc"
gQuadTimFlags	equ	gQuadTim+4
