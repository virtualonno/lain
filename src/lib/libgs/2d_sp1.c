#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "libgs_int.h"

/* One DMA tag followed by GP0(E1h) and a variable-size textured rectangle. */
typedef struct {
    unsigned long tag;
    unsigned long drawMode;
    unsigned long colorAndCode;
    unsigned long position;
    unsigned long texcoordAndClut;
    unsigned long size;
} FastSpritePacket;

void GsSortFastSprite(GsSPRITE *sp, GsOT *ot, unsigned short pri)
{
    unsigned long attribute = sp->attribute;
    if ((long)attribute >= 0 && sp->w && sp->h) {
        unsigned long drawModeWithPage;
        FastSpritePacket *packet;
        unsigned short spriteX, spriteY;
        int x, y;
        int b, g, r, cx, cy;
        unsigned char u, v;
        unsigned long drawModeBase, drawModeWithColorMode, blendMode;
        unsigned long semiTransparent, spriteOpcode, color;
        GsOT_TAG *tag;
        unsigned long packetAddress;

        packet = (FastSpritePacket *)GsOUT_PACKET_P;
        spriteY = (unsigned short)sp->y;
        spriteX = (unsigned short)sp->x;
        drawModeBase = 0xe1000200;
        packetAddress = (unsigned long)packet & 0xffffff;
        drawModeWithColorMode = ((attribute >> 17) & 0x180) | drawModeBase;
        drawModeWithPage = (sp->tpage & 0x1f) | drawModeWithColorMode;
        blendMode = (attribute >> 23) & 0x60;
        packet->drawMode = drawModeWithPage | blendMode;
        x = spriteX + (unsigned short)GsORGOFSX;
        y = spriteY + (unsigned short)GsORGOFSY;
        b = sp->b;
        g = sp->g;
        r = sp->r;
        packet->position = (x & 0xffff) | ((unsigned long)y << 16);
        semiTransparent = (attribute >> 5) & 0x02000000;
        spriteOpcode = ((attribute << 18) & 0x01000000) | 0x64000000;
        color = semiTransparent | spriteOpcode | (b << 16) | (g << 8);
        packet->colorAndCode = color | r;
        v = sp->v;
        u = sp->u;
        cy = sp->cy;
        cx = sp->cx;
        packet->texcoordAndClut = u | (v << 8) | ((unsigned long)cy << 22)
                               | (((unsigned long)cx << 12) & 0x3f0000);
        packet->size = sp->w | ((unsigned long)sp->h << 16);
        tag = ot->org + pri - ot->offset;
        /* The OT entry has a zero word count; preserve its link in the new tag. */
        packet->tag = *(unsigned long *)tag + (5 << 24);
        *(unsigned long *)tag = packetAddress;
        GsOUT_PACKET_P = (PACKET *)(packet + 1);
    }
}
