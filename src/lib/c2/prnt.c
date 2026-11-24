#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include <strings.h>
#include "c2_int.h"

#define LONGARG  0x01
#define LONGDBL  0x02
#define SHORTARG 0x04
#define ALT      0x08
#define LEFT     0x10
#define ZEROPAD  0x20
#define HEXPFX   0x40

int prnt(int output, const signed char *format, va_list ap)
{
    signed char buf[40];
    int count, dprec, trailZeros, width;
    const char *digits;
    int flags, prec, sign, ch, base, size, fieldsz, realsz, n;
    unsigned long value;
    const signed char *cp;
    const signed char *nul;
    signed char last;

    const signed char *fmt;

    if (!format)
        return 0;
    fmt = format;
    digits = "0123456789abcdef";
    for (count = 0;; ++fmt) {
        ch = *(const unsigned char *)fmt;
        if (ch == '\0') {
            _putchar_flash();
            return count;
        }
        if (ch != '%') {
            _putchar(ch);
            continue;
        }
        flags = 0;
        prec = -1;
        sign = 0;
        dprec = 0;
        trailZeros = 0;
        width = 0;
    rflag:
        switch (*(const unsigned char *)++fmt) {
        case ' ':
            if (!sign)
                sign = ' ';
            goto rflag;
        case '#':
            flags |= ALT;
            goto rflag;
        case '*':
            width = va_arg(ap, int);
            if (width >= 0)
                goto rflag;
            width = -width;
            /* fall through */
        case '-':
            flags |= LEFT;
            goto rflag;
        case '+':
            sign = '+';
            goto rflag;
        case '.':
            if (*(const unsigned char *)++fmt == '*') {
                n = va_arg(ap, int);
            } else {
                n = 0;
                while (isascii(*(const unsigned char *)fmt) && isdigit(*fmt))
                    n = 10 * n + (*(const unsigned char *)fmt++ - '0');
                --fmt;
            }
            prec = n;
            if (n < 0)
                prec = -1;
            goto rflag;
        case '0':
            flags |= ZEROPAD;
            goto rflag;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            n = 0;
            do {
                n = 10 * n + (*(const unsigned char *)fmt++ - '0');
            } while (isascii(*(const unsigned char *)fmt) && isdigit(*fmt));
            width = n;
            --fmt;
            goto rflag;
        case 'L':
            flags |= LONGDBL;
            goto rflag;
        case 'h':
            flags |= SHORTARG;
            goto rflag;
        case 'l':
            flags |= LONGARG;
            goto rflag;
        case 'c':
            cp = buf;
            buf[0] = va_arg(ap, unsigned char);
            size = 1;
            sign = 0;
            goto pforw;
        case 'D':
            flags |= LONGARG;
            /* fall through */
        case 'd': case 'i':
            value = (flags & LONGARG) ? va_arg(ap, long) :
                    (flags & SHORTARG) ? va_arg(ap, short) : va_arg(ap, int);
            if ((long)value < 0) {
                value = -value;
                sign = '-';
            }
            base = 10;
            goto number;
        case 'n':
            if (flags & LONGARG)
                *va_arg(ap, long *) = count;
            else if (flags & SHORTARG)
                *va_arg(ap, short *) = count;
            else
                *va_arg(ap, int *) = count;
            break;
        case 'O':
            flags |= LONGARG;
            /* fall through */
        case 'o':
            value = (flags & LONGARG) ? va_arg(ap, long) :
                    (flags & SHORTARG) ? va_arg(ap, short) : va_arg(ap, int);
            base = 8;
            goto nosign;
        case 'p':
            value = va_arg(ap, unsigned long);
            base = 16;
            goto nosign;
        case 's':
            cp = va_arg(ap, const signed char *);
            if (cp == 0)
                cp = (const signed char *)"(null)";
            if (prec >= 0) {
                if ((nul = memchr((unsigned char *)cp, 0, prec)) == 0 ||
                    (size = nul - cp) > prec)
                    size = prec;
            } else {
                size = strlen((const char *)cp);
            }
            sign = 0;
            goto pforw;
        case 'U':
            flags |= LONGARG;
            /* fall through */
        case 'u':
            value = (flags & LONGARG) ? va_arg(ap, long) :
                    (flags & SHORTARG) ? va_arg(ap, short) : va_arg(ap, int);
            base = 10;
            goto nosign;
        case 'X':
            digits = "0123456789ABCDEF";
            /* fall through */
        case 'x':
            value = (flags & LONGARG) ? va_arg(ap, long) :
                    (flags & SHORTARG) ? va_arg(ap, short) : va_arg(ap, int);
            base = 16;
            if ((flags & ALT) && value != 0)
                flags |= HEXPFX;
        nosign:
            sign = 0;
        number:
            dprec = prec;
            if (dprec >= 0)
                flags &= ~ZEROPAD;
            cp = buf + sizeof(buf);
            if (value != 0 || dprec != 0) {
                do {
                    last = (unsigned char)digits[value % base];
                    value /= base;
                    *(signed char *)--cp = last;
                } while (value != 0);
                digits = "0123456789abcdef";
                if ((flags & ALT) && base == 8 && last != '0')
                    *(signed char *)--cp = '0';
            }
            size = buf + sizeof(buf) - cp;
        pforw:
            fieldsz = size + trailZeros;
            if (sign)
                ++fieldsz;
            if (flags & HEXPFX)
                fieldsz += 2;
            realsz = dprec > fieldsz ? dprec : fieldsz;
            if (!(flags & (LEFT | ZEROPAD)) && width)
                for (n = realsz; n < width; ++n)
                    _putchar(' ');
            if ((ch = sign) != 0)
                _putchar(ch);
            if (flags & HEXPFX) {
                _putchar('0');
                _putchar(*fmt);
            }
            if ((flags & (LEFT | ZEROPAD)) == ZEROPAD)
                for (n = realsz; n < width; ++n)
                    _putchar('0');
            for (n = fieldsz; n < dprec; ++n)
                _putchar('0');
            for (n = size; --n >= 0;)
                _putchar(*cp++);
            while (--trailZeros >= 0)
                _putchar('0');
            if (flags & LEFT)
                for (n = realsz; n < width; ++n)
                    _putchar(' ');
            count += width > realsz ? width : realsz;
            break;
        case '\0':
            _putchar_flash();
            return count;
        default:
            ch = *fmt;
            ++count;
            _putchar(ch);
            break;
        }
    }
}
