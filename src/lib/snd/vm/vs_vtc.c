#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

short SsVabTransCompleted(short immediateFlag)
{
    return SpuIsTransferCompleted(immediateFlag);
}
