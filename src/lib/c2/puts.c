#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "c2_int.h"

/* 7 bytes of the module's data; nothing reads them */
static unsigned char putsData[] = { 0x50, 0x73, 0x02, 0x25, 0x9b, 0x52, 0x43 };

void puts(char *s)
{
    char c;

    if (s == NULL) {
        s = "<NULL>";
    }
    while ((c = *s++) != '\0') {
        _putchar(c);
    }
    _putchar_flash();
}
