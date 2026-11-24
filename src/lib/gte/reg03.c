/* Setters that load GTE data and control registers from their arguments. */

#include "gte_int.h"

void SetVertex0(SVECTOR *v0)
{
    __asm__ volatile ("lwc2 $0, 0(%0)" : : "r"(v0));
    __asm__ volatile ("lwc2 $1, 4(%0)" : : "r"(v0));
}

void SetVertex1(SVECTOR *v1)
{
    __asm__ volatile ("lwc2 $2, 0(%0)" : : "r"(v1));
    __asm__ volatile ("lwc2 $3, 4(%0)" : : "r"(v1));
}

void SetVertex2(SVECTOR *v2)
{
    __asm__ volatile ("lwc2 $4, 0(%0)" : : "r"(v2));
    __asm__ volatile ("lwc2 $5, 4(%0)" : : "r"(v2));
}

void SetVertexTri(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2)
{
    __asm__ volatile ("lwc2 $0, 0(%0)" : : "r"(v0));
    __asm__ volatile ("lwc2 $1, 4(%0)" : : "r"(v0));
    __asm__ volatile ("lwc2 $2, 0(%0)" : : "r"(v1));
    __asm__ volatile ("lwc2 $3, 4(%0)" : : "r"(v1));
    __asm__ volatile ("lwc2 $4, 0(%0)" : : "r"(v2));
    __asm__ volatile ("lwc2 $5, 4(%0)" : : "r"(v2));
}

void SetRGBfifo(CVECTOR *rgb0, CVECTOR *rgb1, CVECTOR *rgb2)
{
    __asm__ volatile ("lwc2 $20, 0(%0)" : : "r"(rgb0));
    __asm__ volatile ("lwc2 $21, 0(%0)" : : "r"(rgb1));
    __asm__ volatile ("lwc2 $22, 0(%0)" : : "r"(rgb2));
}

void SetIR123(long ir1, long ir2, long ir3)
{
    __asm__ volatile ("mtc2 %0, $9" : : "r"(ir1));
    __asm__ volatile ("mtc2 %0, $10" : : "r"(ir2));
    __asm__ volatile ("mtc2 %0, $11" : : "r"(ir3));
}

void SetIR0(long ir0)
{
    __asm__ volatile ("mtc2 %0, $8" : : "r"(ir0));
}

void SetSZfifo3(long sz1, long sz2, long sz3)
{
    __asm__ volatile ("mtc2 %0, $17" : : "r"(sz1));
    __asm__ volatile ("mtc2 %0, $18" : : "r"(sz2));
    __asm__ volatile ("mtc2 %0, $19" : : "r"(sz3));
}

void SetSZfifo4(long sz0, long sz1, long sz2, long sz3)
{
    __asm__ volatile ("mtc2 %0, $16" : : "r"(sz0));
    __asm__ volatile ("mtc2 %0, $17" : : "r"(sz1));
    __asm__ volatile ("mtc2 %0, $18" : : "r"(sz2));
    __asm__ volatile ("mtc2 %0, $19" : : "r"(sz3));
}

void SetSXSYfifo(long sxy0, long sxy1, long sxy2)
{
    __asm__ volatile ("mtc2 %0, $12" : : "r"(sxy0));
    __asm__ volatile ("mtc2 %0, $13" : : "r"(sxy1));
    __asm__ volatile ("mtc2 %0, $14" : : "r"(sxy2));
}

void SetRii(long r11r12, long r22r23, long r33)
{
    __asm__ volatile ("ctc2 %0, $0" : : "r"(r11r12));
    __asm__ volatile ("ctc2 %0, $2" : : "r"(r22r23));
    __asm__ volatile ("ctc2 %0, $4" : : "r"(r33));
}

void SetMAC123(long mac1, long mac2, long mac3)
{
    __asm__ volatile ("mtc2 %0, $25" : : "r"(mac1));
    __asm__ volatile ("mtc2 %0, $26" : : "r"(mac2));
    __asm__ volatile ("mtc2 %0, $27" : : "r"(mac3));
}

void SetData32(long lzcs)
{
    __asm__ volatile ("mtc2 %0, $30" : : "r"(lzcs));
}

void SetDQA(long dqa)
{
    __asm__ volatile ("ctc2 %0, $27" : : "r"(dqa));
}

void SetDQB(long dqb)
{
    __asm__ volatile ("ctc2 %0, $28" : : "r"(dqb));
}
