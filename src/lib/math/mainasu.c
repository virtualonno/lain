/* LIBMATH _mainasu: 64-bit two's-complement mantissa negation for software double arithmetic. */
#include "math_int.h"

DoubleWords _mainasu(DoubleWords words)
{
    DoubleWords one;

    one.high = 0;
    one.low = 1;
    words.high = ~words.high;
    words.low = ~words.low;
    words = _add_mant_d(words, one);
    return words;
}
