/*
 * Internal definitions for LIBMATH software floating-point helpers.
 */
#ifndef MATH_INT_H
#define MATH_INT_H

/* IEEE-754 single-precision float viewed as its 32-bit representation. */
typedef union {
    float value;
    int bits;
} FloatBits;

/* Little-endian IEEE-754 double split into its low and high 32-bit words. */
typedef struct {
    unsigned int low;
    int high;
} DoubleWords;

typedef union {
    double value;
    DoubleWords words;
} DoubleBits;

DoubleWords _dbl_shift(int right, DoubleWords words, int count);
DoubleWords _dbl_shift_us(int right, DoubleWords words, int count);
DoubleWords _add_mant_d(DoubleWords left, DoubleWords right);
DoubleWords _mainasu(DoubleWords words);
int _err_math(int error, int operation);

#endif
