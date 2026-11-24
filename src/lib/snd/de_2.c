/* NRPN low note limit handler (DE_LIMITL) for SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr2(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtGetVagAtr(vab, prog, tone, &attr);
    attr.min = value;
    SsUtSetVagAtr(vab, prog, tone, &attr);
}
