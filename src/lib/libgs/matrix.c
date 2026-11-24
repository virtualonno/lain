#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>

GsCOORDINATE2 *DSTACK[32];

void GsInitCoordinate2(GsCOORDINATE2 *super, GsCOORDINATE2 *base)
{
    base->coord = GsIDMATRIX;
    base->super = super;
    base->flg = 0;
    if (super != WORLD && super != SCREEN)
        base->super->sub = base;
}

void GsInitCoord2param(GsCOORD2PARAM *param)
{
    param->rotate.vx = param->rotate.vy = param->rotate.vz = 0;
    param->scale.vx = param->scale.vy = param->scale.vz = ONE;
    param->trans.vx = param->trans.vy = param->trans.vz = 0;
}

void GsSetLsMatrix(MATRIX *m)
{
    SetRotMatrix(m);
    SetTransMatrix(m);
}

void GsSetLightMatrix(MATRIX *m)
{
    MATRIX light = GsLIGHTWSMATRIX;
    PushMatrix();
    MulMatrix(&light, m);
    PopMatrix();
    SetLightMatrix(&light);
}

void GsSetLightMatrix2(MATRIX *m)
{
    SetLightMatrix(MulMatrix2(&GsLIGHTWSMATRIX, m));
}

void GsMulCoord0(MATRIX *a, MATRIX *b, MATRIX *out)
{
    ApplyMatrixLV(a, (VECTOR *)b->t, (VECTOR *)out->t);
    MulMatrix0(a, b, out);
    out->t[0] += a->t[0];
    out->t[1] += a->t[1];
    out->t[2] += a->t[2];
}

void GsMulCoord2(MATRIX *a, MATRIX *b)
{
    VECTOR t;
    ApplyMatrixLV(a, (VECTOR *)b->t, &t);
    MulMatrix2(a, b);
    b->t[0] = t.vx + a->t[0];
    b->t[1] = t.vy + a->t[1];
    b->t[2] = t.vz + a->t[2];
}

void GsMulCoord3(MATRIX *a, MATRIX *b)
{
    VECTOR t;
    ApplyMatrixLV(a, (VECTOR *)b->t, &t);
    MulMatrix(a, b);
    a->t[0] = t.vx + a->t[0];
    a->t[1] = t.vy + a->t[1];
    a->t[2] = t.vz + a->t[2];
}

void print_matrix(MATRIX *m)
{
    printf("(%d %d %d) (%d %d %d) (%d %d %d)\n",
           m->m[0][0], m->m[0][1], m->m[0][2],
           m->m[1][0], m->m[1][1], m->m[1][2],
           m->m[2][0], m->m[2][1], m->m[2][2]);
}

void print_vector(MATRIX *m)
{
    printf("(%d %d %d)\n", m->t[0], m->t[1], m->t[2]);
}
