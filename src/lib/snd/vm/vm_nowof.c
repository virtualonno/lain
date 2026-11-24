/* Queue key-off for the current voice. */

#include <sys/types.h>
#include "vm_f.h"
#include "vm_nowof.h"

void _SsVmKeyOffNow(short unused)
{
    unsigned short voice;
    unsigned short mask_lo;
    unsigned short mask_hi;

    voice = _svm_cur.voice;
    if (voice < 16) {
        mask_lo = 1 << voice;
        mask_hi = 0;
    } else {
        mask_lo = 0;
        mask_hi = 1 << (voice - 16);
    }

    _svm_voice[voice].keystat = KEYSTAT_OFF;
    _svm_voice[voice].pitch = 0;
    _svm_voice[voice].vag_id = 0;

    _svm_okof1 |= mask_lo;
    _svm_okof2 |= mask_hi;
    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
}
