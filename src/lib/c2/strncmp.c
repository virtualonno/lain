#include <sys/types.h>
#include <strings.h>

int strncmp(char *a, char *b, int count)
{
    char c;

    if (!a || !b) {
        return a == b ? 0 : (a ? 1 : -1);
    }
    while (--count >= 0 && (c = *a) == *b++) {
        ++a;
        if (!c) {
            return 0;
        }
    }
    if (count < 0) {
        return 0;
    }
    return *a - b[-1];
}
