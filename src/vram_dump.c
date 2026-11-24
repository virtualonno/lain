/* Writes a VRAM rectangle to a host file as a TIM: pixelMode selects 4-, 8- or 16-bit pixels
 * (enum TimPixelMode), the first two with a 256-entry CLUT stored from clutX, clutY. width counts
 * pixels, x VRAM halfwords. No routine calls it. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsn.h>
#include "heap.h"
#include "vram_dump.h"

void DumpVramTim(char *name, int pixelMode, int x, int y, int width, int height, int clutX, int clutY)
{
    RECT rect;
    int vramWidth;
    int hasClut;
    int imageBytes;
    int remaining;
    char *image;
    char *cursor;
    int fd;

    switch (pixelMode) {
    case TIM_PIXEL_4BIT:
        if (width & 3) {
            width += 4 - width % 4;
        }
        vramWidth = (width + 3) / 4;
        hasClut = 1;
        break;
    case TIM_PIXEL_8BIT:
        if (width & 1) {
            width = (width / 2) * 2 + 2;
        }
        vramWidth = (width + 1) / 2;
        hasClut = 1;
        break;
    case TIM_PIXEL_16BIT:
        vramWidth = width;
        hasClut = 0;
        break;
    default:
        return;
    }
    imageBytes = ((unsigned)vramWidth * height) << 1;
    remaining = (hasClut * 524 + 20) + imageBytes;
    image = (char *)AllocHeapBlock(remaining);
    if (image == 0) {
        return;
    }
    cursor = image + 8;
    ((u_long *)image)[0] = 16;
    ((u_long *)image)[1] = (hasClut << 3) | pixelMode;
    if (hasClut) {
        ((u_long *)image)[2] = 524;
        ((u_short *)image)[6] = 0;
        ((u_short *)image)[7] = 0;
        ((u_short *)image)[8] = 16;
        ((u_short *)image)[9] = 16;
        setRECT(&rect, clutX, clutY, 256, 1);
        StoreImage2(&rect, (u_long *)(image + 20));
        DrawSync(0);
        cursor = image + 532;
    }
    *(u_long *)cursor = imageBytes + 12;
    cursor += 4;
    *(u_short *)cursor = 0;
    cursor += 2;
    *(u_short *)cursor = 0;
    cursor += 2;
    *(u_short *)cursor = vramWidth;
    cursor += 2;
    *(u_short *)cursor = height;
    setRECT(&rect, x, y, vramWidth, height);
    StoreImage2(&rect, (u_long *)(cursor + 2));
    DrawSync(0);
    fd = PCcreat(name, 0);
    cursor = image;
    while (remaining != 0) {
        if (remaining >= 4096) {
            PCwrite(fd, cursor, 4096);
            remaining -= 4096;
            cursor += 4096;
        } else {
            PCwrite(fd, cursor, remaining);
            remaining = 0;
        }
    }
    PCclose(fd);
    FreeHeapBlock(image);
}
