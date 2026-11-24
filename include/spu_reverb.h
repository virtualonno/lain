#ifndef SPU_REVERB_H
#define SPU_REVERB_H

/* Values for the 32 reverb registers (halfword 0xe0 + n from SPU_REG_BASE); mask bit n selects
 * register n, a mask of 0 selects them all. */
typedef struct {
    unsigned long mask;
    unsigned short dAPF1, dAPF2, vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4, vWALL;
    unsigned short vAPF1, vAPF2, mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    unsigned short dLSAME, dRSAME, mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    unsigned short dLDIFF, dRDIFF, mLAPF1, mRAPF1, mLAPF2, mRAPF2, vLIN, vRIN;
} SpuReverbPreset;

#define SPU_REV_MASK_dAPF1      0x00000001
#define SPU_REV_MASK_dAPF2      0x00000002
#define SPU_REV_MASK_vIIR       0x00000004
#define SPU_REV_MASK_vCOMB1     0x00000008
#define SPU_REV_MASK_vCOMB2     0x00000010
#define SPU_REV_MASK_vCOMB3     0x00000020
#define SPU_REV_MASK_vCOMB4     0x00000040
#define SPU_REV_MASK_vWALL      0x00000080
#define SPU_REV_MASK_vAPF1      0x00000100
#define SPU_REV_MASK_vAPF2      0x00000200
#define SPU_REV_MASK_mLSAME     0x00000400
#define SPU_REV_MASK_mRSAME     0x00000800
#define SPU_REV_MASK_mLCOMB1    0x00001000
#define SPU_REV_MASK_mRCOMB1    0x00002000
#define SPU_REV_MASK_mLCOMB2    0x00004000
#define SPU_REV_MASK_mRCOMB2    0x00008000
#define SPU_REV_MASK_dLSAME     0x00010000
#define SPU_REV_MASK_dRSAME     0x00020000
#define SPU_REV_MASK_mLDIFF     0x00040000
#define SPU_REV_MASK_mRDIFF     0x00080000
#define SPU_REV_MASK_mLCOMB3    0x00100000
#define SPU_REV_MASK_mRCOMB3    0x00200000
#define SPU_REV_MASK_mLCOMB4    0x00400000
#define SPU_REV_MASK_mRCOMB4    0x00800000
#define SPU_REV_MASK_dLDIFF     0x01000000
#define SPU_REV_MASK_dRDIFF     0x02000000
#define SPU_REV_MASK_mLAPF1     0x04000000
#define SPU_REV_MASK_mRAPF1     0x08000000
#define SPU_REV_MASK_mLAPF2     0x10000000
#define SPU_REV_MASK_mRAPF2     0x20000000
#define SPU_REV_MASK_vLIN       0x40000000
#define SPU_REV_MASK_vRIN       0x80000000

void _spu_setReverbAttr(SpuReverbPreset *param);

#endif
