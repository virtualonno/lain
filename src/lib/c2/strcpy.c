#include <sys/types.h>
#include <strings.h>

char *strcpy(char *dest, char *src)
{
    char *start;

    if (!dest || !src) {
        return 0;
    }
    start = dest;
    while ((*dest++ = *src++) != '\0') {
    }
    return start;
}
