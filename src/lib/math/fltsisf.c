/* LIBMATH __floatsisf: software 32-bit signed integer to IEEE-754 float conversion. */
#include "math_int.h"

float __floatsisf(int value)
{
    int sign = 0;
    int exponent = 127 + 30;
    int mantissa;
    FloatBits number;

    if (!value)
        return 0.0f;
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
    value += 64;
    if (value < 0) {
        ++exponent;
        value = (unsigned int)value >> 8;
    } else {
        value = (unsigned int)value >> 7;
    }
    exponent = sign | (exponent << 23);
    mantissa = value & ~0x00800000;
    number.bits = exponent | mantissa;
    return number.value;
}
