	opt	c+,at-,m-
#include <gtereg.h>
	include	"INLINE_A.H"

	xdef	NormalClip

	section	.text

NormalClip:
	mtc2	a0,C2_SXY0
	mtc2	a2,C2_SXY2
	mtc2	a1,C2_SXY1
	nNCLIP
	mfc2	v0,C2_MAC0
	jr	ra
	nop
	nop
	nop
	nop
