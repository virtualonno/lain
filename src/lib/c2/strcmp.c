#include <sys/types.h>
#include <strings.h>

int strcmp(char *a, char *b)
{
    if (!a || !b) {
        return a == b ? 0 : (a ? 1 : -1);
    }
    while ((signed char)*a == (signed char)*b++) {
        if (*a++ == '\0') {
            return 0;
        }
    }
    return (signed char)*a - (signed char)b[-1];
}
