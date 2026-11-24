#include "math_int.h"

float __mulsf3(float left, float right)
{
    FloatBits a, b, zero;
    int sign, aExp, bExp, exponent, exponentSum;
    int aHigh, bHigh, leftCross, rightCross;
    unsigned int product;

    a.value = left;
    b.value = right;
    sign = (a.bits & 0x80000000U) ^ (b.bits & 0x80000000U);
    if (!(a.bits & 0x7fffffff) || !(b.bits & 0x7fffffff)) {
        zero.bits = sign;
        return zero.value;
    }
    aExp = a.bits >> 23;
    a.bits = (a.bits & 0x007fffff) | 0x00800000;
    bExp = b.bits >> 23;
    b.bits = (b.bits & 0x007fffff) | 0x00800000;
    aHigh = a.bits >> 8;
    bHigh = b.bits >> 8;
    product = aHigh * bHigh;
    leftCross = (a.bits & 0xff) * bHigh;
    rightCross = (b.bits & 0xff) * aHigh;
    exponentSum = (aExp & 0xff) + (bExp & 0xff);
    product += leftCross >> 8;
    product += rightCross >> 8;
    exponent = exponentSum - 126;
    if (product & 0x80000000U) {
        product += 128;
        product >>= 8;
    } else {
        product += 64;
        if (product & 0x80000000U) {
            product >>= 8;
        } else {
            product >>= 7;
            exponent = exponentSum - 127;
        }
    }
    product &= ~0x00800000U;
    if (exponent >= 255) {
        unsigned int infinity;

        _err_math(34, 12);
        infinity = 0x7f800000U;
        if (sign)
            infinity = 0xff800000U;
        a.bits = infinity;
    } else {
        a.bits = sign | (exponent << 23) | product;
    }
    return a.value;
}
