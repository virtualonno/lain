#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct {
    u_short low, high;
} TimWord;

void GsGetTimInfo(u_long *im, GsIMAGE *tim)
{
    u_long mode = tim->pmode = *im;
    if ((mode >> 3) & 1) {
        TimWord *pixels;
        im++;
        pixels = (TimWord *)(im + *im / sizeof(*im));
        im++;
        tim->cx = ((TimWord *)im)->low;
        tim->cy = ((TimWord *)im++)->high;
        tim->cw = ((TimWord *)im)->low;
        tim->ch = ((TimWord *)im++)->high;
        tim->clut = im;
        pixels++;
        tim->px = pixels->low;
        tim->py = pixels++->high;
        tim->pw = pixels->low;
        tim->ph = pixels++->high;
        tim->pixel = (u_long *)pixels;
    } else {
        im += 2;
        tim->px = ((TimWord *)im)->low;
        tim->py = ((TimWord *)im++)->high;
        tim->pw = ((TimWord *)im)->low;
        tim->ph = ((TimWord *)im++)->high;
        tim->pixel = im;
    }
}
