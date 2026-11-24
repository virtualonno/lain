/* MDEC run/level codes, run in bits 15-10 and level in bits 9-0: the first 111 codes of a
   code table, then the same codes with their level negated. */
unsigned short gSignedRunLevelCodes[222];

void BuildSignedRunLevelCodes(const unsigned short *codes)
{
    int i;
    unsigned short unused; /* 2 bytes; nothing uses it */
    unsigned short level;

    for (i = 0; i < 111; i++) {
        gSignedRunLevelCodes[i] = codes[i];
        level = codes[i] & 0x3ff;
        level = -level;
        level &= 0x3ff;
        gSignedRunLevelCodes[i + 111] = (codes[i] & 0xfc00) | level;
    }
}
