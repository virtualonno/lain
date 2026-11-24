/* NRPN reverb depth handler (DE_REV_DEPTH) for SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr16(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtSetReverbDepth(value, value);
}
