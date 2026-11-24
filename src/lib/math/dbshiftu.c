/* LIBMATH _dbl_shift_us: unsigned 64-bit shift (zero-filling right shift). */
#include "math_int.h"

DoubleWords _dbl_shift_us(int right, DoubleWords words, int count)
{
    int i;

    if (right == 0) {
        for (i = 0; i < count; ++i) {
            words.high <<= 1;
            if (words.low & 0x80000000U)
                words.high |= 1;
            words.low <<= 1;
        }
    } else {
        for (i = 0; i < count; ++i) {
            words.low >>= 1;
            if (words.high & 1)
                words.low |= 0x80000000U;
            words.high = (unsigned int)words.high >> 1;
        }
    }
    return words;
}
