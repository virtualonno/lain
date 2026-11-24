#ifndef SPU_VM_VOICE_H
#define SPU_VM_VOICE_H

/* _svm_sreg_dirty[] bits: the shadow registers _SsVmFlush copies to the SPU. */
#define SREG_DIRTY_VOL_L        0x01
#define SREG_DIRTY_VOL          0x03
#define SREG_DIRTY_PITCH        0x04
#define SREG_DIRTY_START_ADDR   0x08
#define SREG_DIRTY_ADSR1        0x10
#define SREG_DIRTY_ADSR         0x30

/* SpuVmVoice.keystat */
#define KEYSTAT_OFF             0
#define KEYSTAT_TONE            1
#define KEYSTAT_NOISE           2

/* One voice's state (_svm_voice[24], 0x36 bytes each in LIBSND 4.3). */
typedef struct {
    short vag_id;
    short age;                  /* allocations since it was last assigned */
    short pitch;
    unsigned short envx;
    short vol;
    unsigned char pan;          /* 0x40 = center */
    short channel;
    short note;
    short seq_sep_no;           /* the owner's SpuVmCur.seq_sep_no, -1 for none */
    short prog_index;
    short prog;
    short tone;
    short vab_id;
    short prior;                /* assigned tone priority */
    unsigned char unused;       /* nothing reads it */
    signed char keystat;      /* KEYSTAT_* */
    short auto_vol;             /* nonzero: _SsVmFlush calls _autovol for the voice */
    short auto_vol_state[5];
    short auto_pan;             /* nonzero: _SsVmFlush calls _autopan for the voice */
    short auto_pan_state[5];
} SpuVmVoice;

#endif
