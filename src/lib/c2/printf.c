#include <sys/types.h>
#include <stdarg.h>
#include "c2_int.h"

/* The caller reserves a home slot for each of the four register arguments, and any
 * further arguments follow them on the stack: storing the three after format into
 * their slots makes the argument list va_start points at. */
int printf(format, first, second, third)
char *format;
unsigned long first, second, third;
{
    va_list ap;

    va_start(ap, format);
    ((unsigned long *)ap)[0] = first;
    ((unsigned long *)ap)[1] = second;
    ((unsigned long *)ap)[2] = third;
    return prnt(1, (const signed char *)format, ap);
}
