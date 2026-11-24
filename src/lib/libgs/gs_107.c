#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

MATRIX GsLIGHTWSMATRIX;
MATRIX _LC;

static void SetColor(MATRIX *color);
static void GetColor(MATRIX *color);

int GsSetFlatLight(int id, GsF_LIGHT *light)
{
    MATRIX world, color;
    int length;
    int r = light->r;
    int g = light->g;
    int b = light->b;

    world = GsLIGHTWSMATRIX;
    GetColor(&color);
    length = SquareRoot0(light->vx * light->vx + light->vy * light->vy + light->vz * light->vz);
    if (!length)
        return -1;

    switch (id) {
    case 0:
        world.m[0][0] = -light->vx * ONE / length;
        world.m[0][1] = -light->vy * ONE / length;
        world.m[0][2] = -light->vz * ONE / length;
        color.m[0][0] = r * ONE / 255;
        color.m[1][0] = g * ONE / 255;
        color.m[2][0] = b * ONE / 255;
        break;
    case 1:
        world.m[1][0] = -light->vx * ONE / length;
        world.m[1][1] = -light->vy * ONE / length;
        world.m[1][2] = -light->vz * ONE / length;
        color.m[0][1] = r * ONE / 255;
        color.m[1][1] = g * ONE / 255;
        color.m[2][1] = b * ONE / 255;
        break;
    case 2:
        world.m[2][0] = -light->vx * ONE / length;
        world.m[2][1] = -light->vy * ONE / length;
        world.m[2][2] = -light->vz * ONE / length;
        color.m[0][2] = r * ONE / 255;
        color.m[1][2] = g * ONE / 255;
        color.m[2][2] = b * ONE / 255;
        break;
    }

    GsLIGHTWSMATRIX = world;
    SetColor(&color);
    return 0;
}

static void SetColor(MATRIX *color)
{
    _LC = *color;
    SetColorMatrix(color);
}

static void GetColor(MATRIX *color)
{
    *color = _LC;
}
