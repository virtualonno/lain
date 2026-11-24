#include <sys/types.h>
#include <strings.h>

char *strncpy(char *dest, char *src, int count)
{
    char *start;
    int i;

    if (!dest || !src) {
        return 0;
    }
    start = dest;
    for (i = 0; i < count; i++) {
        if ((*dest++ = *src++) == '\0') {
            for (++i; i < count; ++i) {
                *dest++ = '\0';
            }
            return start;
        }
    }
    return start;
}
