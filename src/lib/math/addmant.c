/* LIBMATH _add_mant_d: 64-bit mantissa addition with carry for software double arithmetic. */
#include "math_int.h"

typedef struct {
    unsigned int low;
    unsigned int high;
} LowWordHalves;

DoubleWords _add_mant_d(DoubleWords left, DoubleWords right)
{
    LowWordHalves a, b;

    /* Carry out of each 16-bit half of the low word. */
    a.high = left.low >> 16;
    a.low = left.low & 0xffff;
    b.high = right.low >> 16;
    b.low = right.low & 0xffff;
    a.low += b.low;
    if (a.low & 0x10000U)
        ++a.high;
    a.high += b.high;
    if (a.high & 0x10000U)
        ++left.high;
    left.low = (a.high << 16) | (a.low & 0xffff);
    left.high += right.high;
    return left;
}
