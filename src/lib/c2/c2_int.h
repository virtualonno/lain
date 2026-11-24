/*
 * Routines LIBC2's modules share that no kit header declares.
 */
#ifndef _C2_INT_H_
#define _C2_INT_H_

#include <stdarg.h>

/* putchar.s */
void _putchar(int ch);
void _putchar_flash(void);

/* prnt.c */
int prnt(int output, const signed char *format, va_list ap);

#endif
