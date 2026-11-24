#include <sys/types.h>
#include <libspu.h>

void SpuSetKeyOnWithAttr(SpuVoiceAttr *attr)
{
    SpuSetVoiceAttr(attr);
    SpuSetKey(SPU_ON, attr->voice);
}
