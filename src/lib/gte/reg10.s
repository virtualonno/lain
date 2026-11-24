	opt	c+,at-,m-
#include <gtereg.h>

	xdef	SetBackColor

	section	.text

SetBackColor:
	sll	a0,a0,4
	sll	a1,a1,4
	sll	a2,a2,4
	ctc2	a0,C2_RBK
	ctc2	a1,C2_GBK
	ctc2	a2,C2_BBK
	jr	ra
	nop
