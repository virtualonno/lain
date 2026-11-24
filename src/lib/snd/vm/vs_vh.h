#ifndef VS_VH_H
#define VS_VH_H

/* Gives the SPU address for a VAB body of size bytes; on failure frees
   slot vabid and the transfer lock itself and gives -1. */
typedef long (*VabAlloc)(unsigned long size, unsigned long sbaddr, short vabid);

short _SsVabOpenHeadWithMode(unsigned char *addr, short vabid, VabAlloc alloc, unsigned long sbaddr);

#endif
