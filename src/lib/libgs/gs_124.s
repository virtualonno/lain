; LIBGS GS_124
	opt	c+,at-,m-

	xref	GsOUT_PACKET_P
	xdef	GsSetWorkBase

	section	.text

GsSetWorkBase:
	lui	at,((GsOUT_PACKET_P)+$8000)>>16
	jr	ra
	sw	a0,((GsOUT_PACKET_P)-((((GsOUT_PACKET_P)+$8000)>>16)<<16))(at)
	nop
