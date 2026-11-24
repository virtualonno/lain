#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>
#include "libgs_int.h"

PACKET *_make_packet(PACKET *packet, GsOT *ot, u_short pri, int words)
{
    u_char count = words;
    long index = pri - ot->offset;
    GsOT_TAG *tag;
    if (index < 0)
        printf("ps_sort_sprite,bg: z resolution overflow\n");
    tag = &ot->org[index];
    *(u_long *)packet = *(u_long *)tag;
    setlen(packet, words);
    *(PACKET **)tag = packet;
    setlen(tag, 0);
    return (PACKET *)((u_long *)packet + (count + 1));
}
