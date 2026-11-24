	.data
	.globl GsADIVZ
	.globl GsADIVW
	.globl GsADIVH
GsADIVZ:
	.word 0
GsADIVW:
	.half 1023
GsADIVH:
	.half 511
	.word 0, 0

	.comm GsOUT_PACKET_P,4
	.comm GsMATE_C,4
	.comm GsLMODE,4
	.comm GsLIGNR,4
	.comm GsLIOFF,4
	.comm GsZOVER,4
	.comm GsBACKC,4
	.comm GsNDIV,4
	.comm GsTRATE,4
	.comm GsTON,4
	.comm GsDISPON,4
