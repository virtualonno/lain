#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

void SsSetMVol(short voll, short volr)
{
    SpuCommonAttr attr;

    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
    /* Scale 0..127 libsnd volume to 0..0x3fff spu master volume. */
    attr.mvol.left = voll * 129;
    attr.mvol.right = volr * 129;
    SpuSetCommonAttr(&attr);
}
