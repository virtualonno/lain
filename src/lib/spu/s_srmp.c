#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

static __inline__ void CopyBytes(const void *src, void *dst, long n)
{
    const unsigned char *s = src;
    unsigned char *d = dst;

    while (n--) {
        *d++ = *s++;
    }
}

long SpuSetReverbModeParam(SpuReverbAttr *attr)
{
    unsigned long mask;
    unsigned long mode;
    long all;
    long mode_set;
    long delay_set;
    long feedback_set;
    long clear_wa;
    long rev_on;
    long delay;
    long half_delay;
    long feedback;
    unsigned short spucnt;
    SpuReverbPreset param;

    rev_on = 0;
    mode_set = 0;
    delay_set = 0;
    clear_wa = 0;
    mask = attr->mask;
    feedback_set = 0;
    all = mask == SPU_ALL;
    param.mask = 0;
    if (all || (mask & SPU_REV_MODE)) {
        mode = attr->mode;
        if (mode & SPU_REV_MODE_CLEAR_WA) {
            mode &= ~SPU_REV_MODE_CLEAR_WA;
            clear_wa = 1;
        }
        if (mode >= SPU_REV_MODE_MAX || _SpuIsInAllocateArea_(_spu_rev_startaddr[mode])) {
            return SPU_ERROR;
        }
        mode_set = 1;
        _spu_rev_attr.mode = mode;
        _spu_rev_offsetaddr = _spu_rev_startaddr[_spu_rev_attr.mode];
        CopyBytes(&_spu_rev_param[_spu_rev_attr.mode], &param, sizeof(SpuReverbPreset));
        switch (_spu_rev_attr.mode) {
        case SPU_REV_MODE_ECHO:
            _spu_rev_attr.feedback = 0x7f;
            _spu_rev_attr.delay = 0x7f;
            break;
        case SPU_REV_MODE_DELAY:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0x7f;
            break;
        default:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0;
            break;
        }
    }
    if (all || (mask & SPU_REV_DELAYTIME)) {
        switch (_spu_rev_attr.mode) {
        case SPU_REV_MODE_ECHO:
        case SPU_REV_MODE_DELAY:
            delay_set = 1;
            if (!mode_set) {
                CopyBytes(&_spu_rev_param[_spu_rev_attr.mode], &param, sizeof(SpuReverbPreset));
                param.mask = SPU_REV_MASK_mLSAME | SPU_REV_MASK_mRSAME |
                             SPU_REV_MASK_mLCOMB1 | SPU_REV_MASK_dLSAME |
                             SPU_REV_MASK_mLAPF1 | SPU_REV_MASK_mRAPF1;
            }
            delay = attr->delay;
            _spu_rev_attr.delay = delay;
            param.mLSAME = (delay << 13) / 127 - param.dAPF1;
            half_delay = (delay << 12) / 127;
            param.mRSAME = half_delay - param.dAPF2;
            param.mLCOMB1 = half_delay + param.mRCOMB1;
            param.dLSAME = half_delay + param.dRSAME;
            param.mLAPF1 = half_delay + param.mLAPF2;
            param.mRAPF1 = half_delay + param.mRAPF2;
            break;
        }
    }
    if (all || (mask & SPU_REV_FEEDBACK)) {
        switch (_spu_rev_attr.mode) {
        case SPU_REV_MODE_ECHO:
        case SPU_REV_MODE_DELAY:
            feedback_set = 1;
            if (!mode_set) {
                if (!delay_set) {
                    CopyBytes(&_spu_rev_param[_spu_rev_attr.mode], &param, sizeof(SpuReverbPreset));
                    param.mask = SPU_REV_MASK_vWALL;
                } else {
                    param.mask |= SPU_REV_MASK_vWALL;
                }
            }
            feedback = attr->feedback;
            _spu_rev_attr.feedback = feedback;
            param.vWALL = (feedback * 0x8100) / 127;
            break;
        }
    }
    if (mode_set) {
        rev_on = (_spu_RXX[SPU_REG_SPUCNT] >> SPUCNT_REVERB_SHIFT) & 1;
        if (rev_on) {
            spucnt = _spu_RXX[SPU_REG_SPUCNT];
            spucnt &= ~SPUCNT_REVERB_ENABLE;
            _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        }
    }
    if (!mode_set) {
        if (all || (mask & SPU_REV_DEPTHL)) {
            _spu_RXX[SPU_REG_RVOL_L] = attr->depth.left;
            _spu_rev_attr.depth.left = attr->depth.left;
        }
        if (all || (mask & SPU_REV_DEPTHR)) {
            _spu_RXX[SPU_REG_RVOL_R] = attr->depth.right;
            _spu_rev_attr.depth.right = attr->depth.right;
        }
    } else {
        _spu_RXX[SPU_REG_RVOL_L] = 0;
        _spu_RXX[SPU_REG_RVOL_R] = 0;
        _spu_rev_attr.depth.left = 0;
        _spu_rev_attr.depth.right = 0;
    }
    if (mode_set || delay_set || feedback_set) {
        _spu_setReverbAttr(&param);
    }
    if (clear_wa) {
        SpuClearReverbWorkArea(_spu_rev_attr.mode);
    }
    if (mode_set) {
        _spu_FsetRXX(SPU_REG_MBASE, _spu_rev_offsetaddr, SPU_FSETRXX_RAW);
        if (rev_on) {
            spucnt = _spu_RXX[SPU_REG_SPUCNT];
            spucnt |= SPUCNT_REVERB_ENABLE;
            _spu_RXX[SPU_REG_SPUCNT] = spucnt;
        }
    }
    return SPU_SUCCESS;
}
