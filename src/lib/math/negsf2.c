/* LIBMATH __negsf2: software IEEE-754 float negation returning +0.0f for zero. */
#include "math_int.h"

float __negsf2(float value)
{
    FloatBits number;

    number.value = value;
    if (number.bits == 0)
        return 0.0f;
    number.bits ^= 0x80000000;
    return number.value;
}
