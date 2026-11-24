/* LIBMATH __fixsfsi: software IEEE-754 float-to-signed-int conversion. */
#include "math_int.h"

int __fixsfsi(float value)
{
    FloatBits number;
    int exponent, shift, mantissa;

    number.value = value;
    if (!(number.bits & 0x7fffffff))
        return number.bits & 0x80000000;
    exponent = (number.bits >> 23) & 255;
    shift = exponent - 157;
    if (shift > 0) {
        if (number.bits >= 0) {
            _err_math(34, 18);
            return 0x7fffffff;
        }
        return 0x80000000;
    }
    mantissa = ((number.bits & 0x007fffff) | 0x00800000) << 7;
    if ((unsigned int)(exponent - 126) < 32 && mantissa)
        mantissa >>= -shift;
    else
        return 0;
    return number.bits < 0 ? -mantissa : mantissa;
}
