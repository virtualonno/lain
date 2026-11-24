#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr1(short vab, short prog, short tone, VagAtr attr, short seq, unsigned char value)
{
    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.mode = value;
    SsUtSetVagAtr(vab, prog, tone, &attr);

    if (value == 0)
        SsUtReverbOff();
    else if (value == 4)
        SsUtReverbOn();
}
