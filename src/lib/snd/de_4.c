/* NRPN linear ADSR attack-rate handler (DE_ADSR_AR_L) for SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_adsr.h"

void _SsSetNrpnVabAttr4(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SndAdsr envelope;

    SsUtGetVagAtr(vab, prog, tone, &attr);
    _SsUtResolveADSR(attr.adsr1, attr.adsr2, &envelope);
    envelope.attack_exponential = 0;
    envelope.attack_rate = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
