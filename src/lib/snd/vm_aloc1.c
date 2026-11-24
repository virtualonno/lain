#include <sys/types.h>
#include "snd_vm_int.h"
#include "vm_f.h"

extern long _snd_vmask;

#define NO_VOICE 99

unsigned char _SsVmAlloc(void)
{
    unsigned char selected = NO_VOICE;
    unsigned short quietest = 0xffff;
    unsigned char candidates = 0;
    unsigned short oldest = 0;
    int fallback = NO_VOICE;
    int voice;
    unsigned short priority = _svm_cur.tone_prior;

    for (voice = 0; (unsigned char)voice < _SsVmMaxVoice; ++voice) {
        if (_snd_vmask & (1 << (unsigned char)voice)) {
            continue;
        }
        if (_svm_voice[(unsigned char)voice].keystat == KEYSTAT_OFF && _svm_voice[(unsigned char)voice].envx == 0) {
            selected = voice;
            break;
        }
        if (_svm_voice[(unsigned char)voice].prior < priority) {
            priority = _svm_voice[(unsigned char)voice].prior;
            fallback = voice;
            quietest = _svm_voice[(unsigned char)voice].envx;
            oldest = _svm_voice[(unsigned char)voice].age;
            candidates = 1;
        } else if (_svm_voice[(unsigned char)voice].prior == priority) {
            ++candidates;
            if (_svm_voice[(unsigned char)voice].envx < quietest) {
                oldest = _svm_voice[(unsigned char)voice].age;
                quietest = _svm_voice[(unsigned char)voice].envx;
                fallback = voice;
            } else if (_svm_voice[(unsigned char)voice].envx == quietest) {
                if (oldest < _svm_voice[(unsigned char)voice].age) {
                    oldest = _svm_voice[(unsigned char)voice].age;
                    fallback = voice;
                }
            }
        }
    }
    if (selected == NO_VOICE) {
        selected = fallback;
        if (candidates == 0) {
            selected = _SsVmMaxVoice;
        }
    }
    if (selected < _SsVmMaxVoice) {
        for (voice = 0; (unsigned char)voice < _SsVmMaxVoice; ++voice) {
            if (!(_snd_vmask & (1 << (unsigned char)voice))) {
                ++_svm_voice[(unsigned char)voice].age;
            }
        }
        _svm_voice[selected].age = 0;
        _svm_voice[selected].prior = _svm_cur.tone_prior;
        _svm_voice[selected].auto_pan = 0;
        _svm_voice[selected].auto_vol = 0;
    }
    return selected;
}
