#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdlib.h>
#include "libgs_int.h"

static void ScaleViewPoints(GsRVIEW2 *view, GsRVIEW2 *scaled);
static long MaxViewCoordinate(GsRVIEW2 *view);
static int BitLength(long value);

int GsSetRefView2(GsRVIEW2 *view)
{
    GsRVIEW2 scaled;
    MATRIX matrices[2];
    MATRIX unused; /* 32 bytes; nothing uses it */
    VECTOR translation;
    long distance, s, c, delta;

    GsWSMATRIX = GsIDMATRIX2;
    gte_rotate_z_matrix(&GsWSMATRIX, -view->rz);
    ScaleViewPoints(view, &scaled);
    distance = SquareRoot0((scaled.vrx - scaled.vpx) * (scaled.vrx - scaled.vpx) +
                           (scaled.vry - scaled.vpy) * (scaled.vry - scaled.vpy) +
                           (scaled.vrz - scaled.vpz) * (scaled.vrz - scaled.vpz));
    if (!distance)
        return 1;

    delta = scaled.vpy - scaled.vry;
    s = -((delta * ONE) / distance);
    delta = SquareRoot0((scaled.vrx - scaled.vpx) * (scaled.vrx - scaled.vpx) +
                        (scaled.vrz - scaled.vpz) * (scaled.vrz - scaled.vpz));
    c = delta * ONE / distance;
    Gssub_make_matrix(&matrices[0], s, c, 'x');
    MulMatrix(&GsWSMATRIX, &matrices[0]);
    if (delta) {
        distance = delta;
        delta = scaled.vrx - scaled.vpx;
        s = (delta * ONE) / distance;
        delta = scaled.vrz - scaled.vpz;
        c = (delta * ONE) / distance;
        Gssub_make_matrix(&matrices[0], -s, c, 'y');
        MulMatrix(&GsWSMATRIX, &matrices[0]);
    }

    translation.vx = -view->vpx;
    translation.vy = -view->vpy;
    translation.vz = -view->vpz;
    ApplyMatrixLV(&GsWSMATRIX, &translation, (VECTOR *)GsWSMATRIX.t);

    if (view->super != WORLD) {
        GsGetLw(view->super, &matrices[0]);
        TransposeMatrix(&matrices[0], &matrices[1]);
        ApplyMatrixLV(&matrices[1], (VECTOR *)matrices[0].t, &translation);
        matrices[1].t[0] = -translation.vx;
        matrices[1].t[1] = -translation.vy;
        matrices[1].t[2] = -translation.vz;
        GsMulCoord2(&GsWSMATRIX, &matrices[1]);
        GsWSMATRIX = matrices[1];
    }
    GsWSMATRIX_ORG = GsWSMATRIX;
    return 0;
}

static void ScaleViewPoints(GsRVIEW2 *view, GsRVIEW2 *scaled)
{
    int bits = BitLength(MaxViewCoordinate(view));
    if (bits > 15) {
        int shift = bits - 15;
        scaled->vpx = view->vpx >> shift;
        scaled->vpy = view->vpy >> shift;
        scaled->vpz = view->vpz >> shift;
        scaled->vrx = view->vrx >> shift;
        scaled->vry = view->vry >> shift;
        scaled->vrz = view->vrz >> shift;
    } else {
        scaled->vpx = view->vpx;
        scaled->vpy = view->vpy;
        scaled->vpz = view->vpz;
        scaled->vrx = view->vrx;
        scaled->vry = view->vry;
        scaled->vrz = view->vrz;
    }
}

static long MaxViewCoordinate(GsRVIEW2 *view)
{
    long max = labs(view->vpx);
    long n = labs(view->vpy);
    if (max < n) max = n;
    n = labs(view->vpz);
    if (max < n) max = n;
    n = labs(view->vrx);
    if (max < n) max = n;
    n = labs(view->vry);
    if (max < n) max = n;
    n = labs(view->vrz);
    if (max < n) max = n;
    return max;
}

static int BitLength(long value)
{
    int bits = 0;
    while (value > 0) {
        value >>= 1;
        bits++;
    }
    return bits;
}
