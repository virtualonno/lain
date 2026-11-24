	opt	c+,at-,m-
#include <gtereg.h>

	xdef	ReadSZfifo4

	section	.text

ReadSZfifo4:
	swc2	C2_SZ0,0(a0)
	swc2	C2_SZ1,0(a1)
	swc2	C2_SZ2,0(a2)
	swc2	C2_SZ3,0(a3)
	jr	ra
	nop
	nop
	nop
