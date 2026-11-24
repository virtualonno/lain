#ifndef SPU_VM_CUR_H
#define SPU_VM_CUR_H

#define SVM_MODE_STEREO         0
#define SVM_MODE_MONO           1
#define TONE_MODE_REVERB        0x04
#define SVM_SEQ_UTILITY         33      /* seq_sep_no of a voice SsUtKeyOnV keys on */
#define SVM_VAG_NOISE           0xff    /* VAG index selecting the SPU's noise generator */

/* VAB slots, and the states of _svm_vab_used[]. */
#define SVM_VAB_MAX             16
#define SVM_VAB_FREE            0
#define SVM_VAB_ACTIVE          1
#define SVM_VAB_HEAD_OPEN       2

/* The voice being set up (_svm_cur, 0x1a bytes). */
typedef struct {
    unsigned char tones;
    unsigned char vab_id;
    signed char note;
    unsigned char fine;
    signed char vol;
    unsigned char pan;
    signed char prog;
    signed char prog_index;     /* the program's block of 16 tones in _svm_tn */
    unsigned char unused[2];    /* nothing reads them */
    signed char prog_mvol;
    unsigned char prog_mpan;
    signed char tone;
    signed char tone_vol;
    unsigned char tone_pan;
    signed char tone_prior;
    unsigned char tone_center;
    unsigned char tone_shift;
    unsigned char tone_mode;    /* TONE_MODE_* */
    short seq_sep_no;           /* sep number in the low byte, seq number in the high byte */
    short vag_idx;              /* 1-based VAG index, or SVM_VAG_NOISE */
    short voice;
} SpuVmCur;

extern SpuVmCur _svm_cur;
extern unsigned char _svm_vab_used[SVM_VAB_MAX];
extern unsigned long _svm_vab_total[SVM_VAB_MAX];
extern unsigned long _svm_vab_start[SVM_VAB_MAX];

#endif
