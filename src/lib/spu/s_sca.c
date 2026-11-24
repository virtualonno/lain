#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

/* nothing reads or writes it */
long SpuCommonError;

void SpuSetCommonAttr(SpuCommonAttr *attr)
{
    unsigned short vol_l;
    unsigned short vol_r;
    unsigned short mode;
    unsigned long mask;
    long is_all;

    vol_l = 0;
    vol_r = 0;
    mask = attr->mask;
    is_all = (mask == SPU_ALL);

    if (is_all || (mask & SPU_COMMON_MVOLL)) {
        if (is_all || (mask & SPU_COMMON_MVOLMODEL)) {
            switch (attr->mvolmode.left) {
            case SPU_VOICE_DIRECT:
                vol_l = attr->mvol.left;
                mode = SPU_VOL_MODE_DIRECT;
                break;
            case SPU_VOICE_LINEARIncN:
                mode = SPU_VOL_SWEEP_LINEAR_INC_N;
                break;
            case SPU_VOICE_LINEARIncR:
                mode = SPU_VOL_SWEEP_LINEAR_INC_R;
                break;
            case SPU_VOICE_LINEARDecN:
                mode = SPU_VOL_SWEEP_LINEAR_DEC_N;
                break;
            case SPU_VOICE_LINEARDecR:
                mode = SPU_VOL_SWEEP_LINEAR_DEC_R;
                break;
            case SPU_VOICE_EXPIncN:
                mode = SPU_VOL_SWEEP_EXP_INC_N;
                break;
            case SPU_VOICE_EXPIncR:
                mode = SPU_VOL_SWEEP_EXP_INC_R;
                break;
            case SPU_VOICE_EXPDec:
                mode = SPU_VOL_SWEEP_EXP_DEC;
                break;
            default:
                vol_l = attr->mvol.left;
                mode = SPU_VOL_MODE_DIRECT;
                break;
            }
        } else {
            vol_l = attr->mvol.left;
            mode = SPU_VOL_MODE_DIRECT;
        }
        if (mode != SPU_VOL_MODE_DIRECT) {
            if (attr->mvol.left > SPU_VOL_SWEEP_RATE_MAX) {
                vol_l = SPU_VOL_SWEEP_RATE_MAX;
            } else if (attr->mvol.left < 0) {
                vol_l = 0;
            } else {
                vol_l = attr->mvol.left;
            }
        }
        vol_l &= SPU_VOL_DIRECT_MASK;
        _spu_RXX[SPU_REG_MVOL_L] = vol_l | mode;
    }

    if (is_all || (mask & SPU_COMMON_MVOLR)) {
        if (is_all || (mask & SPU_COMMON_MVOLMODER)) {
            switch (attr->mvolmode.right) {
            case SPU_VOICE_DIRECT:
                vol_r = attr->mvol.right;
                mode = SPU_VOL_MODE_DIRECT;
                break;
            case SPU_VOICE_LINEARIncN:
                mode = SPU_VOL_SWEEP_LINEAR_INC_N;
                break;
            case SPU_VOICE_LINEARIncR:
                mode = SPU_VOL_SWEEP_LINEAR_INC_R;
                break;
            case SPU_VOICE_LINEARDecN:
                mode = SPU_VOL_SWEEP_LINEAR_DEC_N;
                break;
            case SPU_VOICE_LINEARDecR:
                mode = SPU_VOL_SWEEP_LINEAR_DEC_R;
                break;
            case SPU_VOICE_EXPIncN:
                mode = SPU_VOL_SWEEP_EXP_INC_N;
                break;
            case SPU_VOICE_EXPIncR:
                mode = SPU_VOL_SWEEP_EXP_INC_R;
                break;
            case SPU_VOICE_EXPDec:
                mode = SPU_VOL_SWEEP_EXP_DEC;
                break;
            default:
                vol_r = attr->mvol.right;
                mode = SPU_VOL_MODE_DIRECT;
                break;
            }
        } else {
            vol_r = attr->mvol.right;
            mode = SPU_VOL_MODE_DIRECT;
        }
        if (mode != SPU_VOL_MODE_DIRECT) {
            if (attr->mvol.right > SPU_VOL_SWEEP_RATE_MAX) {
                vol_r = SPU_VOL_SWEEP_RATE_MAX;
            } else if (attr->mvol.right < 0) {
                vol_r = 0;
            } else {
                vol_r = attr->mvol.right;
            }
        }
        vol_r &= SPU_VOL_DIRECT_MASK;
        _spu_RXX[SPU_REG_MVOL_R] = vol_r | mode;
    }

    if (is_all || (mask & SPU_COMMON_CDVOLL)) {
        _spu_RXX[SPU_REG_CDVOL_L] = attr->cd.volume.left;
    }
    if (is_all || (mask & SPU_COMMON_CDVOLR)) {
        _spu_RXX[SPU_REG_CDVOL_R] = attr->cd.volume.right;
    }
    if (is_all || (mask & SPU_COMMON_EXTVOLL)) {
        _spu_RXX[SPU_REG_EXTVOL_L] = attr->ext.volume.left;
    }
    if (is_all || (mask & SPU_COMMON_EXTVOLR)) {
        _spu_RXX[SPU_REG_EXTVOL_R] = attr->ext.volume.right;
    }
    if (is_all || (mask & SPU_COMMON_CDREV)) {
        if (attr->cd.reverb == SPU_OFF) {
            _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_CD_REVERB;
        } else {
            _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_CD_REVERB;
        }
    }
    if (is_all || (mask & SPU_COMMON_CDMIX)) {
        if (attr->cd.mix == SPU_OFF) {
            _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_CD_MIX;
        } else {
            _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_CD_MIX;
        }
    }
    if (is_all || (mask & SPU_COMMON_EXTREV)) {
        if (attr->ext.reverb == SPU_OFF) {
            _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_EXT_REVERB;
        } else {
            _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_EXT_REVERB;
        }
    }
    if (is_all || (mask & SPU_COMMON_EXTMIX)) {
        if (attr->ext.mix == SPU_OFF) {
            _spu_RXX[SPU_REG_SPUCNT] &= ~SPUCNT_EXT_MIX;
        } else {
            _spu_RXX[SPU_REG_SPUCNT] |= SPUCNT_EXT_MIX;
        }
    }
}
