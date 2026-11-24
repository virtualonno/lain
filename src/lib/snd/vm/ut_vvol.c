/* Get and set individual SPU voice volumes (direct and 0..127 scaled). */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_vm_int.h"
#include "vm_f.h"

short SsUtGetDetVVol(short vc, short *voll, short *volr)
{
    if (vc >= 0 && vc < SPU_VOICE_COUNT) {
        *voll = _svm_sreg[vc * SPU_VREG_COUNT + SPU_VREG_VOL_L];
        *volr = _svm_sreg[vc * SPU_VREG_COUNT + SPU_VREG_VOL_R];
        return 0;
    }
    return -1;
}

short SsUtSetDetVVol(short vc, short voll, short volr)
{
    if (vc >= 0 && vc < SPU_VOICE_COUNT) {
        _svm_sreg_buf[vc * SPU_VREG_COUNT + SPU_VREG_VOL_L] = voll;
        _svm_sreg_buf[vc * SPU_VREG_COUNT + SPU_VREG_VOL_R] = volr;
        _svm_sreg_dirty[vc] |= SREG_DIRTY_VOL;
        return 0;
    }
    return -1;
}

short SsUtGetVVol(short vc, short *voll, short *volr)
{
    short l;
    short r;

    if (vc >= 0 && vc < SPU_VOICE_COUNT) {
        l = _svm_sreg[vc * SPU_VREG_COUNT + SPU_VREG_VOL_L];
        r = _svm_sreg[vc * SPU_VREG_COUNT + SPU_VREG_VOL_R];
        /* Scale 0..0x3fff SPU voice volume to 0..127 LIBSND volume. */
        *voll = l / 129;
        *volr = r / 129;
        return 0;
    }
    return -1;
}

short SsUtSetVVol(short vc, short voll, short volr)
{
    short l;
    short r;

    if (vc >= 0 && vc < SPU_VOICE_COUNT) {
        /* Scale 0..127 LIBSND volume to 0..0x3fff SPU voice volume. */
        l = voll * 129;
        r = volr * 129;
        _svm_sreg_buf[vc * SPU_VREG_COUNT + SPU_VREG_VOL_L] = l;
        _svm_sreg_buf[vc * SPU_VREG_COUNT + SPU_VREG_VOL_R] = r;
        _svm_sreg_dirty[vc] |= SREG_DIRTY_VOL;
        return 0;
    }
    return -1;
}
