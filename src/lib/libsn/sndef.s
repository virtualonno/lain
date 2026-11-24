; LIBSN SNDEF: the default stack and RAM sizes.
	opt	c+

	xdef	_stacksize
	xdef	_ramsize

	section	.rdata
	section	.text
	section	.data
	section	.sdata
bss	group	bss
	section	.sbss,bss
	section	.bss,bss

	section	.data

_stacksize:
	dw	$00008000
_ramsize:
	dw	$00800000
