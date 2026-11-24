/* LIBMATH __adddf3: software IEEE-754 double-precision addition. */
#include "math_int.h"

double __adddf3(double left, double right)
{
    DoubleWords aMantissa, bMantissa;
    DoubleWords rounding;
    DoubleBits zero;
    DoubleBits a, b;
    int sign;
    int aExp, bExp;

    zero.words.low = 0;
    zero.words.high = 0;
    sign = 0;
    a.value = left;
    b.value = right;
    if (!(a.words.high & 0x7fffffff) && !a.words.low)
        return b.value;
    if (!(b.words.high & 0x7fffffff) && !b.words.low)
        return a.value;
    aExp = (a.words.high >> 20) & 0x7ff;
    bExp = (b.words.high >> 20) & 0x7ff;
    if (bExp + 54 < aExp)
        return a.value;
    if (aExp + 54 < bExp)
        return b.value;
    aMantissa.high = (a.words.high & 0xfffff) | 0x100000;
    aMantissa.low = a.words.low;
    bMantissa.high = (b.words.high & 0xfffff) | 0x100000;
    bMantissa.low = b.words.low;
    if (a.words.high & 0x80000000)
        aMantissa = _mainasu(aMantissa);
    if ((b.words.high & 0x80000000) != 0)
        bMantissa = _mainasu(bMantissa);
    aMantissa = _dbl_shift(0, aMantissa, 9);
    bMantissa = _dbl_shift(0, bMantissa, 9);
    if (bExp < aExp) {
        bMantissa = _dbl_shift(1, bMantissa, aExp - bExp);
    } else {
        aMantissa = _dbl_shift(1, aMantissa, bExp - aExp);
        aExp = bExp;
    }
    aMantissa = _add_mant_d(aMantissa, bMantissa);
    if (aMantissa.high < 0) {
        aMantissa = _mainasu(aMantissa);
        sign = 0x80000000;
    } else if (!aMantissa.high && !aMantissa.low) {
        return zero.value;
    }
    if (!(aMantissa.high & 0xe0000000)) {
        do {
            aMantissa = _dbl_shift(0, aMantissa, 1);
            --aExp;
        } while (!(aMantissa.high & 0xe0000000));
    }
    if (aMantissa.high & 0x40000000) {
        aMantissa = _dbl_shift(1, aMantissa, 1);
        ++aExp;
    }
    rounding.high = 0;
    rounding.low = (aMantissa.low & 0x200) ? 0x100 : 0xff;
    aMantissa = _add_mant_d(aMantissa, rounding);
    if (aMantissa.high & 0x40000000) {
        aMantissa = _dbl_shift(1, aMantissa, 1);
        ++aExp;
    }
    aMantissa = _dbl_shift(1, aMantissa, 9);
    aMantissa.high &= ~0x100000;
    if (aExp >= 0x7ff) {
        _err_math(34, 11);
        a.words.high = sign ? 0xfff00000 : 0x7ff00000;
        a.words.low = 0;
    } else {
        a.words.high = sign | ((unsigned int)aExp << 20) | aMantissa.high;
        a.words.low = aMantissa.low;
    }
    return a.value;
}
