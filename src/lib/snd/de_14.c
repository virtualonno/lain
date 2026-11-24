#include <sys/types.h>
#include <libsnd.h>

#include "snd_adsr.h"

void _SsSetNrpnVabAttr14(short vab, short prog, short tone, VagAtr attr, short seq, unsigned char value)
{
    SndAdsr envelope;

    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.porW = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
