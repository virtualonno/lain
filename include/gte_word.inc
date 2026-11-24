; GTE commands written as their coprocessor words.

wRTPS		macro
		dw	$4a180001
		endm

wRTPT		macro
		dw	$4a280030
		endm

wDCPL		macro
		dw	$4a680029
		endm

wDPCS		macro
		dw	$4a780010
		endm

wDPCT		macro
		dw	$4af8002a
		endm

wINTPL		macro
		dw	$4a980011
		endm

wNCS		macro
		dw	$4ac8041e
		endm

wNCT		macro
		dw	$4ad80420
		endm

wNCDS		macro
		dw	$4ae80413
		endm

wNCDT		macro
		dw	$4af80416
		endm

wNCCS		macro
		dw	$4b08041b
		endm

wNCCT		macro
		dw	$4b18043f
		endm

wCDP		macro
		dw	$4b280414
		endm

wCC		macro
		dw	$4b38041c
		endm

wAVSZ3		macro
		dw	$4b58002d
		endm

wAVSZ4		macro
		dw	$4b68002e
		endm

wMVMVA		macro	shift,matrix,vector,addend,clamp
		dw	$4a400012|shift<<19|matrix<<17|vector<<15|addend<<13|clamp<<10
		endm

wSQR		macro	shift
		dw	$4aa00428|shift<<19
		endm

wOP		macro	shift
		dw	$4b70000c|shift<<19
		endm

wGPF		macro	shift
		dw	$4b90003d|shift<<19
		endm

wGPL		macro	shift
		dw	$4ba0003e|shift<<19
		endm
