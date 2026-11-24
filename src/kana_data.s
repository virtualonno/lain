; Kana-to-romaji lookup tables for RomanizeKanaText.
	opt	c+
	section	.data

	xdef	gRomajiVowels
	xdef	gRomajiConsonants
	xdef	gRomajiYVowels
	xdef	gRomajiVoicedConsonants

gRomajiVowels:
	db	"AIUEO",0,0,0
gRomajiConsonants:
	db	"KSTNHM",0,0
gRomajiYVowels:
	db	"AUO",0
gRomajiVoicedConsonants:
	db	"GZDBP",0,0,0
