/* NRPN data entry DE_ECHO_FB, installed in SsFCALL.ccentry. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr17(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtSetReverbFeedback(value);
}
