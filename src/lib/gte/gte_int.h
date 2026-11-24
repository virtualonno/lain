/*
 * LIBGTE's internal sine table, which rsin and rcos read, rsin's quadrant step, and the
 * routines that load single GTE registers, which libgte.h does not declare.
 */
#ifndef _GTE_INT_H_
#define _GTE_INT_H_

#include <sys/types.h>
#include <libgte.h>

/*
 * Quarter-wave sine table (defined in sincos.s), angle 4096 per turn, value in 4096ths:
 * round(sin(i * 2*pi / 4096) * 4096) for i = 0..1024 (0..90 deg).
 */
extern short rsin_tbl[1025];

/* Sine of an angle in [0, 4095] by quadrant from rsin_tbl (geo_00.c). */
extern int sin_1(int a);

/* reg03.c */
void SetVertex0(SVECTOR *v0);
void SetVertex1(SVECTOR *v1);
void SetVertex2(SVECTOR *v2);
void SetVertexTri(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2);
void SetRGBfifo(CVECTOR *rgb0, CVECTOR *rgb1, CVECTOR *rgb2);
void SetIR123(long ir1, long ir2, long ir3);
void SetIR0(long ir0);
void SetSZfifo3(long sz1, long sz2, long sz3);
void SetSZfifo4(long sz0, long sz1, long sz2, long sz3);
void SetSXSYfifo(long sxy0, long sxy1, long sxy2);
void SetRii(long r11r12, long r22r23, long r33);
void SetMAC123(long mac1, long mac2, long mac3);
void SetData32(long lzcs);
void SetDQA(long dqa);
void SetDQB(long dqb);

#endif /* _GTE_INT_H_ */
