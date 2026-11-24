#ifndef SND_VM_INT_H
#define SND_VM_INT_H

#include <libspu.h>
#include <libsnd.h>

#include "psx_hw.h"
#include "spu_vm_cur.h"
#include "spu_vm_voice.h"

/* VAB table limits. */
#define VAB_TONES_PER_PROG      16
#define VAB_MAX_VAGS            256

/* Envelope history depth and full envelope level. */
#define SVM_ENVX_HIST_MAX       16
#define SVM_ENVX_MAX            0x7fff

extern short _svm_vab_count;
extern short kMaxPrograms;
extern VabHdr *_svm_vh;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;
extern VabHdr *_svm_vab_vh[SVM_VAB_MAX];
extern ProgAtr *_svm_vab_pg[SVM_VAB_MAX];
extern VagAtr *_svm_vab_tn[SVM_VAB_MAX];
extern SpuReverbAttr _svm_rattr;

extern signed char _SsVmMaxVoice;
extern signed char _svm_auto_kof_mode;
extern short _svm_stereo_mono;
extern short _svm_damper;

#endif /* SND_VM_INT_H */
