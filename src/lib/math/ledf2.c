#include "math_int.h"

int __ledf2(double left, double right)
{
    DoubleBits a, b;
    int aSign, aExp, bExp, aMantissa, bMantissa;

    a.value = left;
    b.value = right;
    if (a.words.high == b.words.high && a.words.low == b.words.low)
        return 0;
    if (!(a.words.high & 0x7fffffff) && !a.words.low &&
        !(b.words.high & 0x7fffffff) && !b.words.low)
        return 0;
    aSign = a.words.high & 0x80000000;
    if (aSign != (b.words.high & 0x80000000))
        return !aSign;
    aExp = (a.words.high >> 20) & 2047;
    bExp = (b.words.high >> 20) & 2047;
    if (aExp < bExp) {
        return (a.words.high | 0x7fffffff) == -1;
    }
    if (aExp == bExp) {
        aMantissa = (a.words.high & 0x000fffff) | 0x00100000;
        bMantissa = (b.words.high & 0x000fffff) | 0x00100000;
        if (aMantissa < bMantissa ||
            (aMantissa == bMantissa && a.words.low <= b.words.low))
            return (a.words.high | 0x7fffffff) == -1;
    }
    if (!(a.words.high & 0x80000000))
        return 1;
    return 0;
}
