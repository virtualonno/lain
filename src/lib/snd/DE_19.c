/* LIBSND DE_19: NRPN DE_DELAY (SsFCALL.ccentry[DE_DELAY]) handler; sets SPU reverb delay. */

#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr19(short vab, short prog, short tone, VagAtr attr, short nrpn_lsb, unsigned char value)
{
    SsUtSetReverbDelay(value);
}
