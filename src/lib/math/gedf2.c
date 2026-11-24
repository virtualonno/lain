#include "math_int.h"

int __gedf2(double left, double right)
{
    DoubleBits a, b;
    int aSign, aExp, bExp;

    a.value = left;
    b.value = right;
    if ((a.words.high == b.words.high && a.words.low == b.words.low) ||
        (!(a.words.high & 0x7fffffff) && !a.words.low &&
         !(b.words.high & 0x7fffffff) && !b.words.low))
        return 0;
    aSign = a.words.high & 0x80000000;
    if (aSign != (b.words.high & 0x80000000)) {
        if (aSign)
            return -1;
        return 0;
    }
    aExp = (a.words.high >> 20) & 2047;
    bExp = (b.words.high >> 20) & 2047;
    if (aExp > bExp ||
        (aExp == bExp &&
         (((a.words.high & 0x000fffff) | 0x00100000) > ((b.words.high & 0x000fffff) | 0x00100000) ||
          (((a.words.high & 0x000fffff) | 0x00100000) == ((b.words.high & 0x000fffff) | 0x00100000) &&
           a.words.low >= b.words.low)))) {
        if (a.words.high < 0)
            return -1;
        else
            return 0;
    }
    if (a.words.high < 0)
        return 0;
    return -1;
}
