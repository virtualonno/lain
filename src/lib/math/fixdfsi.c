/* LIBMATH __fixdfsi: software IEEE-754 double-to-signed-int conversion. */
#include "math_int.h"

int __fixdfsi(double value)
{
    DoubleBits number;
    DoubleWords mantissa;
    int exponent, shift, result;

    number.value = value;
    if (!(number.words.high & 0x7fffffff) && !number.words.low)
        return number.words.high & 0x80000000;
    exponent = (number.words.high >> 20) & 0x7ff;
    shift = exponent - 1053;
    if (shift > 0) {
        _err_math(34, 17);
        return number.words.high < 0 ? 0x80000000 : 0x7fffffff;
    }
    mantissa.high = (number.words.high & 0xfffff) | 0x100000;
    mantissa.low = number.words.low;
    mantissa = _dbl_shift(0, mantissa, 10);
    result = mantissa.high;
    if ((unsigned int)(exponent - 1022) < 32 && result)
        result >>= -shift;
    else
        return 0;
    return number.words.high < 0 ? -result : result;
}
