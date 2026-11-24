/* NRPN data-entry handler for SsFCALL.ccentry[DE_ADSR_SR_E]. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_adsr.h"

void _SsSetNrpnVabAttr9(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SndAdsr envelope;

    SsUtGetVagAtr(vab, prog, tone, &attr);
    _SsUtResolveADSR(attr.adsr1, attr.adsr2, &envelope);
    envelope.sustain_exponential = 1;
    envelope.sustain_rate = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
