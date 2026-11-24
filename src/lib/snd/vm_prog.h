#ifndef VM_PROG_H
#define VM_PROG_H

int _SsVmSetProgVol(short vab_id, short prog, unsigned char volume);
int _SsVmGetProgVol(short vab_id, short prog);
int _SsVmSetProgPan(short vab_id, short prog, unsigned char pan);
int _SsVmGetProgPan(short vab_id, short prog);

#endif
