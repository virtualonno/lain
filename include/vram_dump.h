#ifndef VRAM_DUMP_H
#define VRAM_DUMP_H

enum TimPixelMode {
    TIM_PIXEL_4BIT,
    TIM_PIXEL_8BIT,
    TIM_PIXEL_16BIT
};

void DumpVramTim(char *name, int pixelMode, int x, int y, int width, int height, int clutX, int clutY);

#endif
