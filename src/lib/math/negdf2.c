/* LIBMATH __negdf2: software IEEE-754 double-precision negation. */
#include "math_int.h"

double __negdf2(double value)
{
    DoubleBits number;

    number.value = value;
    if (number.words.high == 0 && number.words.low == 0)
        return value;
    number.words.high ^= 0x80000000U;
    return number.value;
}
