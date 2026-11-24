; LIBAPI A69
	opt	c+,at-,m-
	include	"kernel.inc"

	xdef	erase

	section	.text

erase:
	bios	$b0,$45
