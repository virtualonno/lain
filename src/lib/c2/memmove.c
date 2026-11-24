/* Copy n bytes between regions that may overlap. Returns dst, which the forward
   copy (dst below src) has advanced past the bytes copied. */
#include <memory.h>

void *memmove(unsigned char *dst, unsigned char *src, int n)
{
    if (dst >= src) {
        while (n-- > 0) {
            dst[n] = src[n];
        }
    } else {
        while (n-- > 0) {
            *dst++ = *src++;
        }
    }
    return dst;
}
