#include "math_int.h"

float __addsf3(float left, float right)
{
    FloatBits a, b;
    unsigned int sign = 0;
    unsigned int aExp, bExp;
    int aMantissa, bMantissa;

    a.value = left;
    b.value = right;
    if (!(a.bits & 0x7fffffff))
        return right;
    if (!(b.bits & 0x7fffffff))
        return left;
    aExp = (a.bits >> 23) & 255;
    bExp = (b.bits >> 23) & 255;
    if (bExp + 25 < aExp)
        return left;
    if (aExp + 25 < bExp)
        return right;
    aMantissa = ((a.bits & 0x007fffff) | 0x00800000) << 6;
    bMantissa = ((b.bits & 0x007fffff) | 0x00800000) << 6;
    if (a.bits & 0x80000000U)
        aMantissa = -aMantissa;
    if (b.bits & 0x80000000U)
        bMantissa = -bMantissa;
    if (bExp < aExp) {
        bMantissa >>= aExp - bExp;
    } else {
        aMantissa >>= bExp - aExp;
        aExp = bExp;
    }
    aMantissa += bMantissa;
    if (aMantissa < 0) {
        aMantissa = -aMantissa;
        sign = 0x80000000;
    } else if (!aMantissa) {
        return 0.0f;
    }
    while (!(aMantissa & 0xe0000000)) {
        aMantissa <<= 1;
        --aExp;
    }
    if (aMantissa & 0x40000000) {
        aMantissa >>= 1;
        ++aExp;
    }
    aMantissa = (aMantissa & 64) ? aMantissa + 32 : aMantissa + 31;
    if (aMantissa & 0x40000000) {
        aMantissa >>= 1;
        ++aExp;
    }
    aMantissa >>= 6;
    aMantissa &= ~0x00800000;
    if (aExp >= 255) {
        _err_math(34, 10);
        a.bits = sign ? 0xff800000 : 0x7f800000;
    } else {
        a.bits = sign | (aExp << 23) | aMantissa;
    }
    return a.value;
}
