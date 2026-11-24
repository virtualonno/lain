#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

typedef union {
    u_long word;
    struct { short x, y; } xy;
    struct { u_char r, g, b, code; } color;
} PolyWord;

void GsSortPoly(void *packet, GsOT *ot, u_short depth)
{
    PolyWord *in = packet;
    PolyWord *out = (PolyWord *)GsOUT_PACKET_P;
    int words = 4;
    u_char code = in[1].color.code;
    int gouraud = code & 0x10;
    int quad = code & 8;
    int textured = code & 4;

    out[1].word = in[1].word;
    out[2].xy.x = in[2].xy.x + GsORGOFSX;
    out[2].xy.y = in[2].xy.y + GsORGOFSY;
    if (textured) {
        out[3].word = in[3].word;
        in++;
        out++;
        words = 5;
    }
    if (gouraud) {
        out[3].word = in[3].word;
        in++;
        out++;
        words++;
    }
    out[3].xy.x = in[3].xy.x + GsORGOFSX;
    out[3].xy.y = in[3].xy.y + GsORGOFSY;
    if (textured) {
        out[4].word = in[4].word;
        in++;
        out++;
        words++;
    }
    if (gouraud) {
        out[4].word = in[4].word;
        in++;
        out++;
        words++;
    }
    out[4].xy.x = in[4].xy.x + GsORGOFSX;
    out[4].xy.y = in[4].xy.y + GsORGOFSY;
    if (textured) {
        out[5].word = in[5].word;
        in++;
        out++;
        words++;
    }
    if (quad) {
        if (gouraud) {
            out[5].word = in[5].word;
            in++;
            out++;
            words++;
        }
        out[5].xy.x = in[5].xy.x + GsORGOFSX;
        out[5].xy.y = in[5].xy.y + GsORGOFSY;
        words++;
        if (textured) {
            out[6].word = in[6].word;
            words++;
        }
    }
    GsOUT_PACKET_P = _make_packet(GsOUT_PACKET_P, ot, depth, words);
}
