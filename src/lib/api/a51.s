; LIBAPI A51
	opt	c+,at-,m-
	include	"kernel.inc"

	xdef	lseek

	section	.text

lseek:
	bios	$b0,$33
