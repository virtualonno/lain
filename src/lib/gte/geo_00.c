/*
 * rsin and its quadrant step sin_1: the sine of an angle of 4096 per turn, in 4096ths,
 * from the quarter-wave rsin_tbl.
 */

#include <sys/types.h>
#include <libgte.h>
#include "gte_int.h"

int rsin(int a)
{
    if (a < 0) {
        return -sin_1(-a & 0xfff);
    } else {
        return sin_1(a & 0xfff);
    }
}

int sin_1(int a)
{
    if (a <= 0x800) {
        if (a <= 0x400) {
            return rsin_tbl[a];
        } else {
            return rsin_tbl[0x800 - a];
        }
    } else {
        if (a <= 0xc00) {
            return -rsin_tbl[a - 0x800];
        } else {
            return -rsin_tbl[0x1000 - a];
        }
    }
}
