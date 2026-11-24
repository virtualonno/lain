#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

void SsSetSerialAttr(char s_num, char type, char mode)
{
    SpuCommonAttr c_attr;

    if (s_num == SS_SERIAL_A) {
        if (type == SS_MIX) {
            c_attr.mask = SPU_COMMON_CDMIX;
            c_attr.cd.mix = mode;
        }
        if (type == SS_REV) {
            c_attr.mask = SPU_COMMON_CDREV;
            c_attr.cd.reverb = mode;
        }
    }
    if (s_num == SS_SERIAL_B) {
        if (type == SS_MIX) {
            c_attr.mask = SPU_COMMON_EXTMIX;
            c_attr.ext.mix = mode;
        }
        if (type == SS_REV) {
            c_attr.mask = SPU_COMMON_EXTREV;
            c_attr.ext.reverb = mode;
        }
    }
    SpuSetCommonAttr(&c_attr);
}
