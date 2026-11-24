#include <sys/types.h>
#include "snd_vm_int.h"

int _SsVmSelectToneAndVag(unsigned char *tones, unsigned char *vags)
{
    unsigned char count = 0;
    signed char tone;

    for (tone = 0; tone < (signed char)_svm_cur.tones; ++tone) {
        if (_svm_tn[(signed char)_svm_cur.prog_index * VAB_TONES_PER_PROG + tone].min <= (signed char)_svm_cur.note &&
            (signed char)_svm_cur.note <= _svm_tn[(signed char)_svm_cur.prog_index * VAB_TONES_PER_PROG + tone].max) {
            vags[count] = _svm_tn[(signed char)_svm_cur.prog_index * VAB_TONES_PER_PROG + tone].vag;
            tones[count++] = tone;
        }
    }
    return count;
}
