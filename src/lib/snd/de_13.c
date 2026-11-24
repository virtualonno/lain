/* NRPN data-entry handler for SsFCALL.ccentry[DE_VIB_TIME]. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_adsr.h"

void _SsSetNrpnVabAttr13(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SndAdsr envelope;

    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.vibT = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
