/* rcos: the cosine of an angle of 4096 per turn, in 4096ths, from the quarter-wave rsin_tbl. */

#include <sys/types.h>
#include <libgte.h>
#include "gte_int.h"

int rcos(int a)
{
    if (a < 0) {
        a = -a;
    }
    a &= 0xfff;
    if (a <= 0x800) {
        if (a <= 0x400) {
            return rsin_tbl[0x400 - a];
        } else {
            return -rsin_tbl[a - 0x400];
        }
    } else {
        if (a <= 0xc00) {
            return -rsin_tbl[0xc00 - a];
        } else {
            return rsin_tbl[a - 0xc00];
        }
    }
}
