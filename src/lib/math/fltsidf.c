#include "math_int.h"

double __floatsidf(int value)
{
    unsigned int sign = 0;
    unsigned int exponent = 1023 + 30;
    DoubleWords mantissa;
    DoubleBits result;

    if (value == 0) {
        result.words.low = 0;
        result.words.high = 0;
    } else {
        /* Never returns for INT_MIN, whose negation stays negative. */
        if (value < 0) {
            sign = 0x80000000;
            value = -value;
        }
        while (value <= 0x00ffffff) {
            value <<= 4;
            exponent -= 4;
        }
        while (value <= 0x3fffffff) {
            value <<= 1;
            --exponent;
        }
        /* Move the leading one from bit 30 to bit 20 before packing. */
        mantissa.high = value;
        mantissa.low = 0;
        mantissa = _dbl_shift_us(1, mantissa, 10);
        mantissa.high &= ~0x00100000;
        result.words.high = mantissa.high | sign;
        result.words.high |= exponent << 20;
        result.words.low = mantissa.low;
    }
    return result.value;
}
