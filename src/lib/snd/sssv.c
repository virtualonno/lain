#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

void SsSetSerialVol(char s_num, short voll, short volr)
{
    SpuCommonAttr c_attr;

    if (s_num == SS_SERIAL_A) {
        c_attr.mask = SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
        if (voll >= 128) {
            voll = 127;
        }
        if (volr >= 128) {
            volr = 127;
        }
        c_attr.cd.volume.left = voll * 258;
        c_attr.cd.volume.right = volr * 258;
    }
    if (s_num == SS_SERIAL_B) {
        c_attr.mask = SPU_COMMON_EXTVOLL | SPU_COMMON_EXTVOLR;
        if (voll >= 128) {
            voll = 127;
        }
        if (volr >= 128) {
            volr = 127;
        }
        c_attr.ext.volume.left = voll * 258;
        c_attr.ext.volume.right = volr * 258;
    }
    SpuSetCommonAttr(&c_attr);
}
