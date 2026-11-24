/* LIBMATH __lesf2: software IEEE-754 float comparison returning 0 if left <= right, 1 if left > right. */
#include "math_int.h"

int __lesf2(float left, float right)
{
    FloatBits a, b;
    int aSign, aExp, bExp;

    a.value = left;
    b.value = right;
    if (a.bits == b.bits ||
        (!(a.bits & 0x7fffffff) && !(b.bits & 0x7fffffff)))
        return 0;
    aSign = a.bits & 0x80000000;
    if (aSign != (b.bits & 0x80000000))
        return !aSign;
    aExp = (a.bits >> 23) & 255;
    bExp = (b.bits >> 23) & 255;
    if (aExp < bExp ||
        (aExp == bExp && (((a.bits & 0x007fffff) | 0x00800000) <=
                          ((b.bits & 0x007fffff) | 0x00800000))))
        return aSign != 0;
    if (!(a.bits & 0x80000000))
        return 1;
    return 0;
}
