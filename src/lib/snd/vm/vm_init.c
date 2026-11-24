/* Initialize the LIBSND voice manager and SPU sound RAM allocator. */
#include <sys/types.h>
#include <libspu.h>

#include "snd_vm_int.h"
#include "spu_int.h"
#include "vm_f.h"
#include "vm_init.h"
#include "vm_nowof.h"

/* One leading slot is skipped; SpuMalloc can use one more for its end marker. */
#define VM_MALLOC_RECORDS 32
static SpuMemBlock vm_malloc_table[VM_MALLOC_RECORDS + 2];

void _SsVmInit(char max_voice)
{
    int requested;
    unsigned short i;
    SpuVoiceAttr attr;

    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(VM_MALLOC_RECORDS, (char *)&vm_malloc_table[1]);

    for (i = 0; i < SPU_VOICE_COUNT * SPU_VREG_COUNT; i++) {
        _svm_sreg_buf[i] = 0;
    }
    for (i = 0; i < SPU_VOICE_COUNT; i++) {
        _svm_sreg_dirty[i] = 0;
    }
    _svm_vab_count = 0;
    for (i = 0; i < SVM_VAB_MAX; i++) {
        _svm_vab_used[i] = SVM_VAB_FREE;
    }

    requested = max_voice;
    if ((unsigned int)requested >= SPU_VOICE_COUNT) {
        _SsVmMaxVoice = SPU_VOICE_COUNT;
    } else {
        _SsVmMaxVoice = requested;
    }

    attr.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA |
                SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
    attr.pitch = 0x1000;
    attr.addr = 0x1000;
    attr.adsr1 = 0x80ff;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.adsr2 = 0x4000;

    for (i = 0; i < _SsVmMaxVoice; i++) {
        _svm_voice[i].age = SPU_VOICE_COUNT;
        _svm_voice[i].vag_id = 0xff;
        _svm_voice[i].keystat = KEYSTAT_OFF;
        _svm_voice[i].pitch = 0;
        _svm_voice[i].envx = 0;
        _svm_voice[i].seq_sep_no = -1;
        _svm_voice[i].prog_index = 0;
        _svm_voice[i].prog = 0;
        _svm_voice[i].tone = 0xff;
        _svm_voice[i].vol = 0;
        _svm_voice[i].channel = 0;
        _svm_voice[i].pan = 0x40;
        _svm_voice[i].auto_vol = 0;
        _svm_voice[i].auto_vol_state[0] = 0;
        _svm_voice[i].auto_vol_state[1] = 0;
        _svm_voice[i].auto_vol_state[2] = 0;
        _svm_voice[i].auto_pan = 0;
        _svm_voice[i].auto_pan_state[0] = 0;
        _svm_voice[i].auto_pan_state[1] = 0;
        _svm_voice[i].auto_pan_state[2] = 0;
        _svm_voice[i].auto_pan_state[3] = 0;
        _svm_voice[i].auto_vol_state[3] = 0;

        attr.voice = 1 << i;
        SpuSetVoiceAttr(&attr);
        _svm_cur.voice = i;
        _SsVmKeyOffNow(1);
    }

    _svm_rattr.mask = 0;
    _svm_rattr.depth.left = 0x3fff;
    _svm_rattr.depth.right = 0x3fff;
    _svm_rattr.mode = SPU_REV_MODE_OFF;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    _svm_orev1 = 0;
    _svm_orev2 = 0;
    _svm_onos1 = 0;
    _svm_onos2 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = SVM_MODE_STEREO;
    kMaxPrograms = 128;
    _SsVmFlush();
}
