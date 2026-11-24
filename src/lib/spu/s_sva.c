#include "spu_int.h"

void SpuSetVoiceAttr(SpuVoiceAttr *arg)
{
    long voice;
    unsigned long mask;
    long all;
    unsigned short *center_note;
    volatile long delay;
    volatile unsigned long acc;

    voice = 0;
    mask = arg->mask;
    center_note = _spu_voice_centerNote;
    all = (mask == SPU_ALL);
    do {
        unsigned long voice_bit = 1 << voice;

        if (arg->voice & voice_bit) {
            long reg = voice * SPU_VREG_COUNT;
            unsigned int rate;
            unsigned int mode;
            unsigned int adsr_word;
            unsigned long voice_reg_addr;

            if (all || (mask & SPU_VOICE_PITCH)) {
                _spu_RXX[reg + SPU_VREG_PITCH] = arg->pitch;
            }
            if (all || (mask & SPU_VOICE_SAMPLE_NOTE)) {
                *center_note = arg->sample_note;
            }
            if (all || (mask & SPU_VOICE_NOTE)) {
                unsigned short center = *center_note;
                unsigned short note = arg->note;
                unsigned short pitch = _spu_note2pitch(center >> 8, center & 0xff,
                                                        note >> 8, note & 0xff);
                _spu_RXX[reg + SPU_VREG_PITCH] = pitch;
            }
            if (all || (mask & SPU_VOICE_VOLL)) {
                int volume = arg->volume.left & SPU_VOL_DIRECT_MASK;
                int mode = SPU_VOL_MODE_DIRECT;

                if (all || (mask & SPU_VOICE_VOLMODEL)) {
                    switch (arg->volmode.left) {
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
                    }
                }
                if (mode != SPU_VOL_MODE_DIRECT) {
                    if (arg->volume.left > SPU_VOL_SWEEP_RATE_MAX) {
                        volume = SPU_VOL_SWEEP_RATE_MAX;
                    } else if (arg->volume.left < 0) {
                        volume = 0;
                    }
                }
                _spu_RXX[reg + SPU_VREG_VOL_L] = volume | mode;
            }
            if (all || (mask & SPU_VOICE_VOLR)) {
                int volume = arg->volume.right & SPU_VOL_DIRECT_MASK;
                int mode = SPU_VOL_MODE_DIRECT;

                if (all || (mask & SPU_VOICE_VOLMODER)) {
                    switch (arg->volmode.right) {
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
                    }
                }
                if (mode != SPU_VOL_MODE_DIRECT) {
                    if (arg->volume.right > SPU_VOL_SWEEP_RATE_MAX) {
                        volume = SPU_VOL_SWEEP_RATE_MAX;
                    } else if (arg->volume.right < 0) {
                        volume = 0;
                    }
                }
                _spu_RXX[reg + SPU_VREG_VOL_R] = volume | mode;
            }
            if (all || (mask & SPU_VOICE_WDSA)) {
                _spu_FsetRXXa(reg | SPU_VREG_START_ADDR, arg->addr);
            }
            if (all || (mask & SPU_VOICE_LSAX)) {
                _spu_FsetRXXa(reg | SPU_VREG_LOOP_ADDR, arg->loop_addr);
            }
            if (all || (mask & SPU_VOICE_ADSR_ADSR1)) {
                _spu_RXX[reg + SPU_VREG_ADSR1] = arg->adsr1;
            }
            if (all || (mask & SPU_VOICE_ADSR_ADSR2)) {
                _spu_RXX[reg + SPU_VREG_ADSR2] = arg->adsr2;
            }
            if (all || (mask & SPU_VOICE_ADSR_AR)) {
                rate = arg->ar;

                if (rate > 0x7f) {
                    rate = 0x7f;
                }
                mode = 0;
                if (all || (mask & SPU_VOICE_ADSR_AMODE)) {
                    if (arg->a_mode == SPU_VOICE_EXPIncN) {
                        mode = 0x80;
                    }
                }
                voice_reg_addr = reg * sizeof(*_spu_RXX);
                voice_reg_addr += (unsigned long)_spu_RXX;
                adsr_word = ((volatile unsigned short *)voice_reg_addr)[SPU_VREG_ADSR1];
                adsr_word &= 0xff;
                adsr_word |= (rate | mode) << 8;
                ((volatile unsigned short *)voice_reg_addr)[SPU_VREG_ADSR1] = adsr_word;
            }
            if (all || (mask & SPU_VOICE_ADSR_DR)) {
                rate = arg->dr;

                if (rate > 0xf) {
                    rate = 0xf;
                }
                _spu_RXX[reg + SPU_VREG_ADSR1] =
                    (_spu_RXX[reg + SPU_VREG_ADSR1] & 0xff0f) | (rate << 4);
            }
            if (all || (mask & SPU_VOICE_ADSR_SR)) {
                rate = arg->sr;

                if (rate > 0x7f) {
                    rate = 0x7f;
                }
                mode = 0x100;
                if (all || (mask & SPU_VOICE_ADSR_SMODE)) {
                    switch (arg->s_mode) {
                    case SPU_VOICE_LINEARIncN:
                        mode = 0;
                        break;
                    case SPU_VOICE_EXPIncN:
                        mode = 0x200;
                        break;
                    case SPU_VOICE_EXPDec:
                        mode = 0x300;
                        break;
                    }
                }
                voice_reg_addr = reg * sizeof(*_spu_RXX);
                voice_reg_addr += (unsigned long)_spu_RXX;
                adsr_word = ((volatile unsigned short *)voice_reg_addr)[SPU_VREG_ADSR2];
                adsr_word &= 0x3f;
                adsr_word |= (rate | mode) << 6;
                ((volatile unsigned short *)voice_reg_addr)[SPU_VREG_ADSR2] = adsr_word;
            }
            if (all || (mask & SPU_VOICE_ADSR_RR)) {
                rate = arg->rr;

                if (rate > 0x1f) {
                    rate = 0x1f;
                }
                mode = 0;
                if (all || (mask & SPU_VOICE_ADSR_RMODE)) {
                    switch (arg->r_mode) {
                    case SPU_VOICE_LINEARDecN:
                        mode = 0;
                        break;
                    case SPU_VOICE_EXPDec:
                        mode = 0x20;
                        break;
                    }
                }
                _spu_RXX[reg + SPU_VREG_ADSR2] =
                    (_spu_RXX[reg + SPU_VREG_ADSR2] & 0xffc0) | (rate | mode);
            }
            if (all || (mask & SPU_VOICE_ADSR_SL)) {
                rate = arg->sl;

                if (rate > 0xf) {
                    rate = 0xf;
                }
                _spu_RXX[reg + SPU_VREG_ADSR1] =
                    (_spu_RXX[reg + SPU_VREG_ADSR1] & 0xfff0) | rate;
            }
        }
        voice++;
        center_note++;
    } while (voice < SPU_VOICE_COUNT);

    /* The SPU needs time between register accesses: busy-wait after the voice writes */
    acc = 1;
    for (delay = 0; delay < 2; delay++) {
        acc *= 13;
    }
}
