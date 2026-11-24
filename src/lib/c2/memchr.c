#include <sys/types.h>
#include <memory.h>

void *memchr(unsigned char *s, unsigned char value, int count)
{
    if (!s || count <= 0) {
        return 0;
    }
    while (--count >= 0) {
        if (*s++ == value) {
            return s - 1;
        }
    }
    return 0;
}
