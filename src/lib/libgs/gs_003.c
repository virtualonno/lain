#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

void GsSetDrawBuffClip(void)
{
    int buf, dx, dy;
    short h, w;
    int x, y;
    RECT *clip = &CLIP2;
    DRAWENV *env = &GsDRAWENV;
    buf = PSDIDX;
    h = clip->h;
    dx = PSDOFSX[buf];
    dy = PSDOFSY[buf];
    w = clip->w;
    env->clip.h = h;
    env->clip.w = w;
    x = clip->x;
    y = clip->y;
    env->clip.x = x + dx;
    env->clip.y = y + dy;
    PutDrawEnv(env);
}
