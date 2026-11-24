; PlayStation TIM binary data objects in original link order.
	opt	c+
	section	.data

	xdef	gLainIconTim
gLainIconTim:
	include	"content/gLainIconTim.inc"

	xdef	gLainDarkIconTim
gLainDarkIconTim:
	include	"content/gLainDarkIconTim.inc"

	xdef	gToukoIconTim
gToukoIconTim:
	include	"content/gToukoIconTim.inc"

	xdef	gLabIconTim
gLabIconTim:
	include	"content/gLabIconTim.inc"

	xdef	gSaiseikunIconTim
gSaiseikunIconTim:
	include	"content/gSaiseikunIconTim.inc"

	xdef	gTransferIconTim
gTransferIconTim:
	include	"content/gTransferIconTim.inc"

	xdef	gMovieIconTim
gMovieIconTim:
	include	"content/gMovieIconTim.inc"

	xdef	gPsychiatryIconTim
gPsychiatryIconTim:
	include	"content/gPsychiatryIconTim.inc"

	xdef	gFlyingCellTim
	xdef	gFlyingCellTimFlags
gFlyingCellTim:
	include	"content/gFlyingCellTim.inc"
gFlyingCellTimFlags	equ	gFlyingCellTim+4

	xdef	gCursorHighlightTim
	xdef	gCursorHighlightTimFlags
gCursorHighlightTim:
	include	"content/gCursorHighlightTim.inc"
gCursorHighlightTimFlags	equ	gCursorHighlightTim+4

	xdef	gOpenedLainIconTim
gOpenedLainIconTim:
	include	"content/gOpenedLainIconTim.inc"

	xdef	gOpenedLainDarkIconTim
gOpenedLainDarkIconTim:
	include	"content/gOpenedLainDarkIconTim.inc"

	xdef	gOpenedToukoIconTim
gOpenedToukoIconTim:
	include	"content/gOpenedToukoIconTim.inc"

	xdef	gOpenedLabIconTim
gOpenedLabIconTim:
	include	"content/gOpenedLabIconTim.inc"

	xdef	gOpenedSaiseikunIconTim
gOpenedSaiseikunIconTim:
	include	"content/gOpenedSaiseikunIconTim.inc"

	xdef	gOpenedTransferIconTim
gOpenedTransferIconTim:
	include	"content/gOpenedTransferIconTim.inc"

	xdef	gOpenedMovieIconTim
gOpenedMovieIconTim:
	include	"content/gOpenedMovieIconTim.inc"

	xdef	gOpenedPsychiatryIconTim
gOpenedPsychiatryIconTim:
	include	"content/gOpenedPsychiatryIconTim.inc"
