#include <sys/types.h>
#include <libsnd.h>

#include "snd_adsr.h"

void _SsSetNrpnVabAttr12(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SndAdsr envelope;

    SsUtGetVagAtr(vab, prog, tone, &attr);
    if (value >= 1 && value <= 63) {
        envelope.sustain_decreasing = 0;
    } else if (value >= 64 && value <= 127) {
        envelope.sustain_decreasing = 1;
    }
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
