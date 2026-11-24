/* LIBSN WRITE: writes arbitrary lengths to a host file in 0x8000-byte chunks
 * through _SN_write, whose leading argument is zero for the BREAK $106 ABI. */
#include <libsn.h>

#define SN_WRITE_LIMIT 0x8000

extern int _SN_write(int, int, int, char *);

int PCwrite(int fd, char *buffer, int length)
{
    int written = 0;

    while (length) {
        int chunk = length;
        int result;

        if ((unsigned int)length > SN_WRITE_LIMIT)
            chunk = SN_WRITE_LIMIT;
        result = _SN_write(0, fd, chunk, buffer);
        if (result == -1)
            return -1;
        written += result;
        buffer += result;
        length -= result;
        if (result < chunk)
            return written;
    }
    return written;
}
