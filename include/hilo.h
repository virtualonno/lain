/* An address's halves: HI for a lui, LO the signed low half the next instruction adds to it. */
#ifndef _HILO_H
#define _HILO_H

#define HI(a)	(((a)+32768)>>16)
#define LO(a)	((a)-(HI(a)<<16))

#endif /* _HILO_H */
