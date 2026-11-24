/* LIBMATH __gesf2: software IEEE-754 float >= comparison (0 if left >= right, -1 otherwise). */
#include "math_int.h"

int __gesf2(float left, float right)
{
    FloatBits a, b;
    int aSign, aExp, bExp;

    a.value = left;
    b.value = right;
    if (a.bits == b.bits ||
        (!(a.bits & 0x7fffffff) && !(b.bits & 0x7fffffff)))
        return 0;
    aSign = a.bits & 0x80000000;
    if (aSign != (b.bits & 0x80000000)) {
        if (aSign)
            return -1;
        return 0;
    }
    aExp = (a.bits >> 23) & 255;
    bExp = (b.bits >> 23) & 255;
    if (aExp > bExp ||
        (aExp == bExp &&
         (((a.bits & 0x007fffff) | 0x00800000) >=
          ((b.bits & 0x007fffff) | 0x00800000)))) {
        if (aSign)
            return -1;
        return 0;
    }
    if (a.bits < 0)
        return 0;
    return -1;
}
