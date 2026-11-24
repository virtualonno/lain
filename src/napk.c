/* napk: unpacks data tagged "napk" into a heap block (tag, 32-bit unpacked size, then flag bytes
 * read MSB first: 0 = literal byte, 1 = 2-byte back-reference with distance-1 and length-3). */
#include <sys/types.h>
#include <strings.h>
#include "heap.h"
#include "napk.h"

char *gNapkTag = "napk";
int gUnusedNapkWord1 = -1; /* 4 bytes; nothing reads or writes it */
int gUnusedNapkWord2 = -1; /* 4 bytes; nothing reads or writes it */

void *UnpackNapk(u_char *packed, int packedSize)
{
    u_char *output;
    int remaining, written;

    if (strncmp((char *)packed, gNapkTag, 4) != 0) {
        return 0;
    }
    packed += 4;
    remaining = *(const int *)packed;
    packed += 4;
    packedSize -= 8;
    output = (u_char *)AllocHeapBlock(remaining);
    if (!output) {
        return 0;
    }
    written = 0;
    while (packedSize != 0 && remaining != 0) {
        u_char flags;
        int bit;

        --packedSize;
        flags = *packed++;
        bit = 0;
        while (packedSize > 0 && remaining > 0) {
            if (flags & (0x80 >> bit)) {
                int i;
                int distance = *packed++;
                int length = *packed++;
                distance++;
                length += 3;
                for (i = 0; i < length; i++) {
                    *(output + written + i) = *(output + written - distance + i);
                }
                written += length;
                packedSize -= 2;
                remaining -= length;
            } else {
                u_char *dst = output + written++;
                u_char value = *packed++;
                --packedSize;
                --remaining;
                *dst = value;
            }
            if (++bit >= 8) {
                break;
            }
        }
    }
    return output;
}
