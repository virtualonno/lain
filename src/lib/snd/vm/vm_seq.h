#ifndef SND_VM_SEQ_H
#define SND_VM_SEQ_H

short _SsVmGetSeqVol(short seq_sep_no, unsigned short *voll, unsigned short *volr);
short _SsVmSetSeqVol(short seq_sep_no, unsigned short voll, unsigned short volr, short unused);
void _SsVmSeqKeyOff(short seq_sep_no);

#endif
