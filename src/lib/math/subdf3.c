/* LIBMATH __subdf3: software IEEE-754 double-precision subtraction. */
#include "math_int.h"

double __subdf3(double left, double right)
{
    DoubleBits a, b;

    b.value = right;
    a.value = left;
    if (!(b.words.high & 0x7fffffff) && !b.words.low)
        return left;
    if (!(a.words.high & 0x7fffffff) && !a.words.low)
        return -b.value;
    b.words.high ^= 0x80000000U;
    return left + b.value;
}
