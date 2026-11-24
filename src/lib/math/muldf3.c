#include "math_int.h"

DoubleWords _mul_mant_d(unsigned int left, unsigned int right);

double __muldf3(double left, double right)
{
    DoubleBits a, b;
    DoubleWords product, rounding;
    DoubleWords leftMantissa, rightMantissa, shifted, crossProduct;
    DoubleBits result;
    unsigned int sign;
    int exponent, exponentSum;
    /* kept: the original keeps the raw sum in memory, in an 8-byte, 8-aligned local */
    DoubleBits savedExponent;
    unsigned int leftHigh, rightHigh, leftLow, rightLow;

    a.value = left;
    b.value = right;
    sign = (a.words.high & 0x80000000U) ^ (b.words.high & 0x80000000U);
    if ((!(a.words.high & 0x7fffffff) && !a.words.low) ||
        (!(b.words.high & 0x7fffffff) && !b.words.low)) {
        result.words.high = sign;
        result.words.low = 0;
        return result.value;
    }
    exponentSum = ((a.words.high >> 20) & 0x7ff) + ((b.words.high >> 20) & 0x7ff);
    exponent = exponentSum - 1022;
    savedExponent.words.low = exponentSum;
    leftMantissa.high = (a.words.high & 0xfffff) | 0x100000;
    leftMantissa.low = a.words.low;
    rightMantissa.high = (b.words.high & 0xfffff) | 0x100000;
    rightMantissa.low = b.words.low;
    shifted = _dbl_shift_us(1, leftMantissa, 21);
    leftHigh = shifted.low;
    shifted = _dbl_shift_us(1, rightMantissa, 21);
    rightHigh = shifted.low;
    leftLow = leftMantissa.low & 0x1fffff;
    rightLow = rightMantissa.low & 0x1fffff;
    product = _mul_mant_d(leftHigh, rightHigh);
    crossProduct = _mul_mant_d(leftLow, rightHigh);
    shifted = _dbl_shift_us(1, crossProduct, 21);
    product = _add_mant_d(product, shifted);
    crossProduct = _mul_mant_d(rightLow, leftHigh);
    shifted = _dbl_shift_us(1, crossProduct, 21);
    product = _add_mant_d(product, shifted);
    if (product.high & 0x80000000U) {
        rounding.high = 0;
        rounding.low = 1024;
        product = _add_mant_d(product, rounding);
        product = _dbl_shift_us(1, product, 11);
    } else {
        rounding.high = 0;
        rounding.low = 512;
        product = _add_mant_d(product, rounding);
        if (product.high & 0x80000000U) {
            product = _dbl_shift_us(1, product, 11);
        } else {
            product = _dbl_shift_us(1, product, 10);
            exponent = savedExponent.words.low - 1023;
        }
    }
    product.high &= ~0x100000U;
    if (exponent >= 2047) {
        _err_math(34, 13);
        result.words.high = sign ? 0xfff00000U : 0x7ff00000U;
        result.words.low = 0;
    } else {
        result.words.high = sign | ((unsigned int)exponent << 20) | product.high;
        result.words.low = product.low;
    }
    return result.value;
}

DoubleWords _mul_mant_d(unsigned int left, unsigned int right)
{
    DoubleWords product;
    DoubleWords crossProduct;
    unsigned int leftLow, rightLow, cross;

    leftLow = left & 0xffff;
    rightLow = right & 0xffff;
    product.high = 0;
    product.low = leftLow * rightLow;
    left >>= 16;
    cross = left * rightLow;
    crossProduct.high = cross >> 16;
    crossProduct.low = cross << 16;
    product = _add_mant_d(product, crossProduct);
    right >>= 16;
    cross = leftLow * right;
    crossProduct.high = cross >> 16;
    crossProduct.low = cross << 16;
    product = _add_mant_d(product, crossProduct);
    product.high += left * right;
    return product;
}
