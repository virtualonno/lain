/* LIBMATH __subsf3: software IEEE-754 single-precision subtraction. */
#include "math_int.h"

float __subsf3(float left, float right)
{
    FloatBits a, b;

    b.value = right;
    a.value = left;
    if (!(b.bits & 0x7fffffff))
        return left;
    if (!(a.bits & 0x7fffffff))
        return -right;
    b.bits ^= 0x80000000U;
    return left + b.value;
}
