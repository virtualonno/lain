#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr3(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.max = value;
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
