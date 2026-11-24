/* The voice manager's buffered SPU registers and key masks, and the flush that
   writes them to the SPU and marks off voices whose envelope has stayed at zero. */
#include <sys/types.h>
#include <libspu.h>

#include "snd_vm_int.h"
#include "vm_f.h"

unsigned short _svm_sreg_buf[SPU_VOICE_COUNT * SPU_VREG_COUNT];
unsigned char _svm_sreg_dirty[SPU_VOICE_COUNT];
unsigned short _svm_okon1;
unsigned short _svm_okon2;
unsigned short _svm_okof1;
unsigned short _svm_okof2;
unsigned short _svm_orev1;
unsigned short _svm_orev2;
unsigned short _svm_onos1;
unsigned short _svm_onos2;
SpuVmVoice _svm_voice[SPU_VOICE_COUNT];
long _svm_envx_ptr;
unsigned long _svm_envx_hist[SVM_ENVX_HIST_MAX];
void (*_autovol)(int);
void (*_autopan)(int);

/* A voice mask has sixteen low voices and eight high voices. */
#define VOICE_MASK(lo, hi) (((unsigned long)(unsigned char)(hi) << 16) | (lo))

void _SsVmFlush(void)
{
    int i;
    unsigned long silent;
    unsigned long bit;
    short noise_lo;
    unsigned char noise_hi;
    SpuVoiceAttr attr;

    _svm_envx_ptr = (_svm_envx_ptr + 1) & (SVM_ENVX_HIST_MAX - 1);
    _svm_envx_hist[_svm_envx_ptr] = 0;

    for (i = 0; i < _SsVmMaxVoice; i++) {
        SpuGetVoiceEnvelope(i, (short *)&_svm_voice[i].envx);
        if (_svm_voice[i].envx == 0) {
            _svm_envx_hist[_svm_envx_ptr] |= 1 << i;
        }
    }

    if (_svm_auto_kof_mode == 0) {
        silent = ~0;
        for (i = 0; i < SVM_ENVX_HIST_MAX - 1; i++) {
            silent &= _svm_envx_hist[i];
        }
        for (i = 0; i < _SsVmMaxVoice; i++) {
            bit = 1 << i;
            if (silent & bit) {
                if (_svm_voice[i].keystat == KEYSTAT_NOISE) {
                    if (i < 16) {
                        noise_lo = bit;
                        noise_hi = 0;
                    } else {
                        noise_lo = 0;
                        noise_hi = 1 << (i - 16);
                    }
                    /* noise_lo is signed: for voice 15 the mask also covers voices 16 to 23. */
                    SpuSetNoiseVoice(SPU_OFF, ((unsigned long)noise_hi << 16) | noise_lo);
                }
                _svm_voice[i].keystat = KEYSTAT_OFF;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;

    for (i = 0; i < SPU_VOICE_COUNT; i++) {
        if (_svm_voice[i].auto_vol) {
            (*_autovol)(i);
        }
        if (_svm_voice[i].auto_pan) {
            (*_autopan)(i);
        }
    }

    for (i = 0; i < SPU_VOICE_COUNT; i++) {
        attr.mask = 0;
        attr.voice = 1 << i;
        if (_svm_sreg_dirty[i] & SREG_DIRTY_VOL_L) {
            attr.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR;
            attr.volume.left = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_VOL_L];
            attr.volume.right = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_VOL_R];
        }
        if (_svm_sreg_dirty[i] & SREG_DIRTY_PITCH) {
            attr.mask |= SPU_VOICE_PITCH;
            attr.pitch = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_PITCH];
        }
        if (_svm_sreg_dirty[i] & SREG_DIRTY_START_ADDR) {
            attr.mask |= SPU_VOICE_WDSA;
            /* Shadow addresses are in eight-byte units; the SDK takes bytes. */
            attr.addr = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_START_ADDR] << 3;
        }
        if (_svm_sreg_dirty[i] & SREG_DIRTY_ADSR1) {
            attr.mask |= SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
            attr.adsr1 = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_ADSR1];
            attr.adsr2 = _svm_sreg_buf[i * SPU_VREG_COUNT + SPU_VREG_ADSR2];
        }
        if (attr.mask) {
            SpuSetVoiceAttr(&attr);
        }
        _svm_sreg_dirty[i] = 0;
    }

    SpuSetKey(SPU_OFF, VOICE_MASK(_svm_okof1, _svm_okof2));
    SpuSetKey(SPU_ON, VOICE_MASK(_svm_okon1, _svm_okon2));
    SpuSetReverbVoice(SPU_BIT, VOICE_MASK(_svm_orev1, _svm_orev2));
    SpuSetNoiseVoice(SPU_BIT, VOICE_MASK(_svm_onos1, _svm_onos2));

    _svm_okof1 = 0;
    _svm_okof2 = 0;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_onos1 = 0;
    _svm_onos2 = 0;
}
