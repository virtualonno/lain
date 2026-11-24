/* NRPN echo delay handler (DE_ECHO_DELAY) for SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr18(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtSetReverbDelay(value);
}
