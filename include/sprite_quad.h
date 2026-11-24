#ifndef SPRITE_QUAD_H
#define SPRITE_QUAD_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

typedef struct {
    POLY_FT4 poly;
    u_long attribute;
    u_short tpage;
    u_short clut;
    u_short scale[2];
    short x;
    short y;
    short w;
    short h;
    short rotateX;
    short rotateY;
    short rotateZ;
    char unused[2]; /* 2 bytes nothing reads */
    u_char u;
    u_char v;
    u_char r;
    u_char g;
    u_char b;
} SpriteQuad;

#endif
