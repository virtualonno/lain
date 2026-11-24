#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

MATRIX GsIDMATRIX, GsIDMATRIX2;
MATRIX GsWSMATRIX, GsWSMATRIX_ORG;

void Gssub_make_matrix(MATRIX *m, short s, short c, char axis)
{
    *m = GsIDMATRIX;
    switch (axis) {
    case 'X':
    case 'x':
        m->m[1][1] = c;
        m->m[2][2] = c;
        m->m[1][2] = -s;
        m->m[2][1] = s;
        break;
    case 'Y':
    case 'y':
        m->m[0][0] = c;
        m->m[2][2] = c;
        m->m[0][2] = s;
        m->m[2][0] = -s;
        break;
    case 'Z':
    case 'z':
        m->m[0][0] = c;
        m->m[1][1] = c;
        m->m[0][1] = -s;
        m->m[1][0] = s;
        break;
    }
}
