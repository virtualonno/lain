#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

GsOT *GsSortOt(GsOT *src, GsOT *dest)
{
    GsOT_TAG *cursor = src->org;
    u_long index = src->point - dest->offset;
    GsOT_TAG *bucket = dest->org;
    GsOT_TAG *previous = cursor;
    GsOT_TAG *splice = cursor;

    while (!isendprim(cursor)) {
        splice = previous;
        previous = cursor;
        cursor = (GsOT_TAG *)cursor->p;
    }
    splice->p = bucket[index].p;
    bucket[index].p = (u_long)src->tag;
    return dest;
}
