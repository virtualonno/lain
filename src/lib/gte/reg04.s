	opt	c+,at-,m-
#include <gtereg.h>

	xdef	ReadSZfifo3

	section	.text

ReadSZfifo3:
	swc2	C2_SZ1,0(a0)
	swc2	C2_SZ2,0(a1)
	swc2	C2_SZ3,0(a2)
	jr	ra
	nop
	nop
	nop
	nop
