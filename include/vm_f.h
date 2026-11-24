#ifndef VM_F_H
#define VM_F_H

#include "psx_hw.h"
#include "snd_vm_int.h"
#include "spu_vm_voice.h"

extern unsigned short *_svm_sreg;
extern unsigned short _svm_sreg_buf[SPU_VOICE_COUNT * SPU_VREG_COUNT];
extern unsigned char _svm_sreg_dirty[SPU_VOICE_COUNT];
extern unsigned short _svm_okon1;
extern unsigned short _svm_okon2;
extern unsigned short _svm_okof1;
extern unsigned short _svm_okof2;
extern unsigned short _svm_onos1;
extern unsigned short _svm_onos2;
extern unsigned short _svm_orev1;
extern unsigned short _svm_orev2;
extern SpuVmVoice _svm_voice[SPU_VOICE_COUNT];
extern long _svm_envx_ptr;
extern unsigned long _svm_envx_hist[SVM_ENVX_HIST_MAX];
extern void (*_autovol)(int);
extern void (*_autopan)(int);

void _SsVmFlush(void);

#endif
