/* Append src to dest. Returns dest, or 0 when either is null or when both strings
   end at the same terminator (one is the tail of the other). */
#include <sys/types.h>
#include <strings.h>

char *strcat(char *dest, char *src)
{
    char *start;

    if (!dest || !src) {
        return 0;
    }
    if (dest + strlen(dest) != src + strlen(src)) {
        start = dest;
        while (*(signed char *)dest++ != '\0') {
        }
        --dest;
        while ((*dest++ = *src++) != '\0') {
        }
        return start;
    }
    return 0;
}
