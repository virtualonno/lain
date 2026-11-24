/* Converts a TMD object table's byte offsets into pointers in place.
 * p points to the TMD flags word (after the id word); bit 0 marks it mapped. */

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

void GsMapModelingData(u_long *p)
{
    int i;
    int n;
    struct TMD_STRUCT *objects;

    if (*p & 1)
        return;
    *p++ |= 1;
    i = 0;
    n = *p++;
    objects = (struct TMD_STRUCT *)p;
    if (n > 0) do {
        objects[i].vertop = (u_long *)((u_long)objects[i].vertop + (u_long)p);
        objects[i].nortop = (u_long *)((u_long)objects[i].nortop + (u_long)p);
        objects[i].primtop = (u_long *)((u_long)objects[i].primtop + (u_long)p);
        i++;
    } while (i < n);
}
