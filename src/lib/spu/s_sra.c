/* Write the selected SPU reverb registers. */
#include <sys/types.h>
#include <libspu.h>

#include "spu_int.h"

/* SPU reverb registers (halfword offsets from 0x1f801c00: 0x1f801dc0..0x1f801dfe). */
#define SPU_REG_REV_dAPF1       0xe0
#define SPU_REG_REV_dAPF2       0xe1
#define SPU_REG_REV_vIIR        0xe2
#define SPU_REG_REV_vCOMB1      0xe3
#define SPU_REG_REV_vCOMB2      0xe4
#define SPU_REG_REV_vCOMB3      0xe5
#define SPU_REG_REV_vCOMB4      0xe6
#define SPU_REG_REV_vWALL       0xe7
#define SPU_REG_REV_vAPF1       0xe8
#define SPU_REG_REV_vAPF2       0xe9
#define SPU_REG_REV_mLSAME      0xea
#define SPU_REG_REV_mRSAME      0xeb
#define SPU_REG_REV_mLCOMB1     0xec
#define SPU_REG_REV_mRCOMB1     0xed
#define SPU_REG_REV_mLCOMB2     0xee
#define SPU_REG_REV_mRCOMB2     0xef
#define SPU_REG_REV_dLSAME      0xf0
#define SPU_REG_REV_dRSAME      0xf1
#define SPU_REG_REV_mLDIFF      0xf2
#define SPU_REG_REV_mRDIFF      0xf3
#define SPU_REG_REV_mLCOMB3     0xf4
#define SPU_REG_REV_mRCOMB3     0xf5
#define SPU_REG_REV_mLCOMB4     0xf6
#define SPU_REG_REV_mRCOMB4     0xf7
#define SPU_REG_REV_dLDIFF      0xf8
#define SPU_REG_REV_dRDIFF      0xf9
#define SPU_REG_REV_mLAPF1      0xfa
#define SPU_REG_REV_mRAPF1      0xfb
#define SPU_REG_REV_mLAPF2      0xfc
#define SPU_REG_REV_mRAPF2      0xfd
#define SPU_REG_REV_vLIN        0xfe
#define SPU_REG_REV_vRIN        0xff

void _spu_setReverbAttr(SpuReverbPreset *param)
{
    unsigned long mask = param->mask;
    long all = mask == 0;

    if (all || (mask & SPU_REV_MASK_dAPF1)) {
        _spu_RXX[SPU_REG_REV_dAPF1] = param->dAPF1;
    }
    if (all || (mask & SPU_REV_MASK_dAPF2)) {
        _spu_RXX[SPU_REG_REV_dAPF2] = param->dAPF2;
    }
    if (all || (mask & SPU_REV_MASK_vIIR)) {
        _spu_RXX[SPU_REG_REV_vIIR] = param->vIIR;
    }
    if (all || (mask & SPU_REV_MASK_vCOMB1)) {
        _spu_RXX[SPU_REG_REV_vCOMB1] = param->vCOMB1;
    }
    if (all || (mask & SPU_REV_MASK_vCOMB2)) {
        _spu_RXX[SPU_REG_REV_vCOMB2] = param->vCOMB2;
    }
    if (all || (mask & SPU_REV_MASK_vCOMB3)) {
        _spu_RXX[SPU_REG_REV_vCOMB3] = param->vCOMB3;
    }
    if (all || (mask & SPU_REV_MASK_vCOMB4)) {
        _spu_RXX[SPU_REG_REV_vCOMB4] = param->vCOMB4;
    }
    if (all || (mask & SPU_REV_MASK_vWALL)) {
        _spu_RXX[SPU_REG_REV_vWALL] = param->vWALL;
    }
    if (all || (mask & SPU_REV_MASK_vAPF1)) {
        _spu_RXX[SPU_REG_REV_vAPF1] = param->vAPF1;
    }
    if (all || (mask & SPU_REV_MASK_vAPF2)) {
        _spu_RXX[SPU_REG_REV_vAPF2] = param->vAPF2;
    }
    if (all || (mask & SPU_REV_MASK_mLSAME)) {
        _spu_RXX[SPU_REG_REV_mLSAME] = param->mLSAME;
    }
    if (all || (mask & SPU_REV_MASK_mRSAME)) {
        _spu_RXX[SPU_REG_REV_mRSAME] = param->mRSAME;
    }
    if (all || (mask & SPU_REV_MASK_mLCOMB1)) {
        _spu_RXX[SPU_REG_REV_mLCOMB1] = param->mLCOMB1;
    }
    if (all || (mask & SPU_REV_MASK_mRCOMB1)) {
        _spu_RXX[SPU_REG_REV_mRCOMB1] = param->mRCOMB1;
    }
    if (all || (mask & SPU_REV_MASK_mLCOMB2)) {
        _spu_RXX[SPU_REG_REV_mLCOMB2] = param->mLCOMB2;
    }
    if (all || (mask & SPU_REV_MASK_mRCOMB2)) {
        _spu_RXX[SPU_REG_REV_mRCOMB2] = param->mRCOMB2;
    }
    if (all || (mask & SPU_REV_MASK_dLSAME)) {
        _spu_RXX[SPU_REG_REV_dLSAME] = param->dLSAME;
    }
    if (all || (mask & SPU_REV_MASK_dRSAME)) {
        _spu_RXX[SPU_REG_REV_dRSAME] = param->dRSAME;
    }
    if (all || (mask & SPU_REV_MASK_mLDIFF)) {
        _spu_RXX[SPU_REG_REV_mLDIFF] = param->mLDIFF;
    }
    if (all || (mask & SPU_REV_MASK_mRDIFF)) {
        _spu_RXX[SPU_REG_REV_mRDIFF] = param->mRDIFF;
    }
    if (all || (mask & SPU_REV_MASK_mLCOMB3)) {
        _spu_RXX[SPU_REG_REV_mLCOMB3] = param->mLCOMB3;
    }
    if (all || (mask & SPU_REV_MASK_mRCOMB3)) {
        _spu_RXX[SPU_REG_REV_mRCOMB3] = param->mRCOMB3;
    }
    if (all || (mask & SPU_REV_MASK_mLCOMB4)) {
        _spu_RXX[SPU_REG_REV_mLCOMB4] = param->mLCOMB4;
    }
    if (all || (mask & SPU_REV_MASK_mRCOMB4)) {
        _spu_RXX[SPU_REG_REV_mRCOMB4] = param->mRCOMB4;
    }
    if (all || (mask & SPU_REV_MASK_dLDIFF)) {
        _spu_RXX[SPU_REG_REV_dLDIFF] = param->dLDIFF;
    }
    if (all || (mask & SPU_REV_MASK_dRDIFF)) {
        _spu_RXX[SPU_REG_REV_dRDIFF] = param->dRDIFF;
    }
    if (all || (mask & SPU_REV_MASK_mLAPF1)) {
        _spu_RXX[SPU_REG_REV_mLAPF1] = param->mLAPF1;
    }
    if (all || (mask & SPU_REV_MASK_mRAPF1)) {
        _spu_RXX[SPU_REG_REV_mRAPF1] = param->mRAPF1;
    }
    if (all || (mask & SPU_REV_MASK_mLAPF2)) {
        _spu_RXX[SPU_REG_REV_mLAPF2] = param->mLAPF2;
    }
    if (all || (mask & SPU_REV_MASK_mRAPF2)) {
        _spu_RXX[SPU_REG_REV_mRAPF2] = param->mRAPF2;
    }
    if (all || (mask & SPU_REV_MASK_vLIN)) {
        _spu_RXX[SPU_REG_REV_vLIN] = param->vLIN;
    }
    if (all || (mask & SPU_REV_MASK_vRIN)) {
        _spu_RXX[SPU_REG_REV_vRIN] = param->vRIN;
    }
}
