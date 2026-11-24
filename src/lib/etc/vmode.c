#include <sys/types.h>
#include <libetc.h>

static long video_mode = MODE_NTSC;

long SetVideoMode(long mode)
{
    long old = video_mode;

    video_mode = mode;
    return old;
}

long GetVideoMode(void)
{
    return video_mode;
}
