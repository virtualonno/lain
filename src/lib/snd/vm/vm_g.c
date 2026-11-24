/* The sound library's voice-manager state: the VAB tables, the voice being set up, reverb and voice limits. */

#include <sys/types.h>
#include <libspu.h>
#include <libsnd.h>

#include "snd_vm_int.h"

signed char _SsVmMaxVoice;
signed char _svm_auto_kof_mode;
short _svm_stereo_mono;
short _svm_damper;
SpuReverbAttr _svm_rattr;
SpuVmCur _svm_cur;

short _svm_vab_count;
short kMaxPrograms;
VabHdr *_svm_vh;
ProgAtr *_svm_pg;
VagAtr *_svm_tn;
unsigned char _svm_vab_used[SVM_VAB_MAX];
VabHdr *_svm_vab_vh[SVM_VAB_MAX];
ProgAtr *_svm_vab_pg[SVM_VAB_MAX];
VagAtr *_svm_vab_tn[SVM_VAB_MAX];
unsigned long _svm_vab_total[SVM_VAB_MAX];
unsigned long _svm_vab_start[SVM_VAB_MAX];
/* 4 bytes; nothing reads or writes it */
long _svm_vg;
