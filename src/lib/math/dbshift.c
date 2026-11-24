#include "math_int.h"

/* A right shift propagates the sign in the high word. */
DoubleWords _dbl_shift(int right, DoubleWords words, int count)
{
    int i;

    if (!right) {
        for (i = 0; i < count; ++i) {
            words.high = (unsigned int)words.high << 1;
            if (words.low & 0x80000000U)
                words.high |= 1;
            words.low <<= 1;
        }
    } else {
        for (i = 0; i < count; ++i) {
            words.low >>= 1;
            if (words.high & 1)
                words.low |= 0x80000000U;
            words.high >>= 1;
        }
    }
    return words;
}
