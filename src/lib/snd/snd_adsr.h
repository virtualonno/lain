#ifndef SND_ADSR_H
#define SND_ADSR_H

/* A voice's adsr1/adsr2 envelope words unpacked, one field per setting;
 * the four flag fields are nonzero when their bit is set. */
typedef struct SndAdsr {
    short attack_rate;
    short decay_rate;
    short sustain_level;
    short sustain_rate;
    short release_rate;
    short attack_exponential;
    short sustain_exponential;
    short release_exponential;
    short sustain_decreasing;
} SndAdsr;

extern void _SsUtResolveADSR(unsigned short adsr1, unsigned short adsr2, SndAdsr *envelope);
extern void _SsUtBuildADSR(SndAdsr *envelope, unsigned short *adsr1, unsigned short *adsr2);

#endif
