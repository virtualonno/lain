#include "math_int.h"

float __divsf3(float numerator, float denominator)
{
    FloatBits a, b, result;
    int difference, exponent, sign, bit, quotient;

    a.value = numerator;
    b.value = denominator;
    difference = ((a.bits >> 23) & 0xff) - ((b.bits >> 23) & 0xff);
    exponent = difference + 126;
    sign = (a.bits & 0x80000000U) ^ (b.bits & 0x80000000U);
    if (!(b.bits & 0x7fffffff)) {
        result.value = 0x7fffffff;
        if (sign)
            result.value = 0xffffffff;
        return result.value;
    }
    if (!(a.bits & 0x7fffffff)) {
        result.bits = sign;
        return result.value;
    }
    a.bits = (a.bits & 0x007fffff) | 0x00800000;
    b.bits = (b.bits & 0x007fffff) | 0x00800000;
    if (a.bits < b.bits) {
        a.bits <<= 1;
        exponent = difference + 125;
    }
    bit = 0x01000000;
    quotient = 0;
    do {
        if (a.bits >= b.bits) {
            quotient |= bit;
            a.bits -= b.bits;
        }
        bit >>= 1;
        a.bits <<= 1;
    } while (bit);
    if (exponent >= 0) {
        ++quotient;
        ++exponent;
        quotient >>= 1;
    } else {
        int shift = -exponent;
        quotient += 1 << shift;
        ++shift;
        quotient >>= shift;
        exponent = 0;
    }
    quotient &= ~0x00800000;
    if (exponent >= 255) {
        _err_math(34, 14);
        quotient = 0x7f800000;
        if (sign)
            quotient = 0xff800000U;
        a.bits = quotient;
    } else {
        a.bits = sign | (exponent << 23) | quotient;
    }
    return a.value;
}
