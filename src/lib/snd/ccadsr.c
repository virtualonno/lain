#include "snd_adsr.h"

void _SsUtResolveADSR(unsigned short adsr1, unsigned short adsr2, SndAdsr *envelope)
{
    envelope->attack_exponential = adsr1 & 0x8000;
    envelope->sustain_exponential = adsr2 & 0x8000;
    envelope->sustain_decreasing = adsr2 & 0x4000;
    envelope->release_exponential = adsr2 & 0x20;

    envelope->attack_rate = (adsr1 >> 8) & 0x7f;
    envelope->decay_rate = (adsr1 >> 4) & 0xf;
    envelope->sustain_level = adsr1 & 0xf;
    envelope->sustain_rate = (adsr2 >> 6) & 0x7f;
    envelope->release_rate = adsr2 & 0x1f;
}

void _SsUtBuildADSR(SndAdsr *envelope, unsigned short *adsr1, unsigned short *adsr2)
{
    unsigned short hi = envelope->sustain_exponential ? 0x8000 : 0;
    unsigned short lo = envelope->attack_exponential ? 0x8000 : 0;

    if (envelope->sustain_decreasing)
        hi |= 0x4000;
    if (envelope->release_exponential)
        hi |= 0x20;

    lo |= (envelope->attack_rate << 8) & 0x7f00;
    lo |= (envelope->decay_rate << 4) & 0xf0;
    lo |= envelope->sustain_level & 0xf;
    hi |= (envelope->sustain_rate << 6) & 0x1fc0;
    hi |= envelope->release_rate & 0x1f;

    *adsr1 = lo;
    *adsr2 = hi;
}
