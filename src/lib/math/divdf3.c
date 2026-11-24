#include "math_int.h"

int _comp_mant(DoubleWords left, DoubleWords right);

double __divdf3(double numerator, double denominator)
{
    DoubleBits a, b, result;
    DoubleWords quotient, bit, rounding;
    DoubleWords dividend, divisor;
    int exponent, sign, shift;

    a.value = numerator;
    b.value = denominator;
    exponent = ((a.words.high >> 20) & 0x7ff) - ((b.words.high >> 20) & 0x7ff) + 1022;
    sign = (a.words.high & 0x80000000U) ^ (b.words.high & 0x80000000U);
    if (!(b.words.high & 0x7fffffff) && !b.words.low) {
        result.words.high = -1;
        if (!sign)
            result.words.high = 0x7fffffff;
        result.words.low = -1;
        return result.value;
    }
    if (!(a.words.high & 0x7fffffff) && !a.words.low) {
        result.words.high = sign;
        result.words.low = 0;
        return result.value;
    }

    dividend.high = (a.words.high & 0xfffff) | 0x100000;
    dividend.low = a.words.low;
    divisor.high = (b.words.high & 0xfffff) | 0x100000;
    divisor.low = b.words.low;
    if (_comp_mant(dividend, divisor) < 0) {
        dividend = _dbl_shift(0, dividend, 1);
        --exponent;
    }
    bit.high = 0x200000;
    bit.low = 0;
    quotient.low = 0;
    quotient.high = 0;
    do {
        if (_comp_mant(dividend, divisor) >= 0) {
            quotient.high |= bit.high;
            quotient.low |= bit.low;
            dividend = _add_mant_d(dividend, _mainasu(divisor));
        }
        dividend = _dbl_shift(0, dividend, 1);
        bit = _dbl_shift_us(1, bit, 1);
    } while (bit.high || bit.low);

    if (exponent >= 0) {
        rounding.high = 0;
        rounding.low = 1;
        quotient = _add_mant_d(quotient, rounding);
        ++exponent;
        quotient = _dbl_shift_us(1, quotient, 1);
    } else {
        shift = -exponent;
        rounding.high = 0;
        rounding.low = 1U << shift;
        quotient = _add_mant_d(quotient, rounding);
        exponent = 0;
        quotient = _dbl_shift_us(1, quotient, shift + 1);
    }
    quotient.high &= ~0x100000;
    if (exponent >= 0x7ff) {
        _err_math(34, 15);
        result.words.high = sign ? 0xfff00000 : 0x7ff00000;
        result.words.low = 0;
    } else {
        result.words.high = sign | (exponent << 20) | quotient.high;
        result.words.low = quotient.low;
    }
    return result.value;
}

int _comp_mant(DoubleWords left, DoubleWords right)
{
    if ((unsigned int)left.high > (unsigned int)right.high)
        return 1;
    if ((unsigned int)left.high < (unsigned int)right.high)
        return -1;
    if (left.low > right.low)
        return 1;
    if (left.low < right.low)
        return -1;
    return 0;
}
