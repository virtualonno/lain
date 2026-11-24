#include <sys/types.h>
#include <libsnd.h>

void _SsSetNrpnVabAttr15(short vab, short prog, short tone, VagAtr attr, short seq, unsigned char value)
{
    SsUtSetReverbType(value);
}
