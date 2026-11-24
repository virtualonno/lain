/* NRPN tone priority handler (DE_PRIORITY) for SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr0(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.prior = value;
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
