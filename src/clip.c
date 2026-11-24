/* LAPKS.BIN clips: each frame's NITR-BS bitstream is unpacked, decoded by the MDEC a 16-pixel
 * column at a time, masked by the frame's LZSS-packed mask and loaded into VRAM. */
#include "loading.h"
#include "INIT.h"
#include "DECODE.h"
#include "clip.h"
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpress.h>

unsigned char *gClipFrameData;
struct ClipFrame {
    int offset;
    short x;
    short y;
    int soundCue;
};
struct ClipChunk {
    short width;
    short height;
    short lumaQuant;
    short chromaQuant;
    int maskOffset; /* bytes from this field to the frame's ClipMask */
    unsigned long bitstream[1];
};
struct ClipMask {
    int unused; /* 4 bytes nothing reads */
    int length;
    unsigned char data[1];
};
struct ClipFrame *gClipFrame;
static unsigned long sRunLevels[32768];
static unsigned long sColumnPixels[2880];
static unsigned char sMaskBuffer[28800];

void DecompressClipMask(void)
{
    unsigned char *chunk;
    unsigned char *src;
    int out;
    int left;
    int bit;
    unsigned char flags;

    unsigned char *buffer;
    int skip;

    chunk = gClipFrameData + gClipFrame->offset;
    out = 0;
    skip = ((struct ClipChunk *)chunk)->maskOffset;
    chunk += 8;
    chunk += skip;
    src = ((struct ClipMask *)chunk)->data;
    left = ((struct ClipMask *)chunk)->length;
    buffer = sMaskBuffer;
    while (left > 0) {
        flags = *src++;
        for (bit = 0; bit < 8; bit++) {
            if (left <= 0)
                return;
            if (flags & (128 >> bit)) {
                int i;
                int distance = *src++;
                int length = *src++;

                length += 3;
                distance++;
                for (i = 0; i < length; i++) {
                    unsigned char *dst = buffer + out;
                    unsigned char *back = dst - distance;
                    dst[i] = back[i];
                }
                out += length;
                left -= length;
            } else {
                unsigned char *dst = buffer + out++;
                unsigned char value = *src++;
                left--;
                *dst = value;
            }
        }
    }
}

static unsigned int sMaskAndBits[16] = {
    0, 0x7fff0000, 0x7fff0000, 0x7fff0000,
    0x00007fff, 0x7fff7fff, 0x7fff7fff, 0x7fff7fff,
    0x00007fff, 0x7fff7fff, 0x7fff7fff, 0x7fff7fff,
    0x00007fff, 0x7fff7fff, 0x7fff7fff, 0x7fff7fff,
};
static unsigned int sMaskOrBits[16] = {
    0, 0x80000000, 0x80000000, 0,
    0x00008000, 0x80008000, 0x80008000, 0x00008000,
    0x00008000, 0x80008000, 0x80008000, 0x00008000,
    0, 0x80000000, 0x80000000, 0,
};

void ApplyClipMask(unsigned int *pixels, int x, int width, int height)
{
    unsigned char *row = sMaskBuffer + x / 4;
    int stride = width / 4;
    int y;

    for (y = 0; y < height; y++) {
        unsigned char *src = row;
        int count;
        for (count = 4; count > 0; count--, src++) {
            unsigned char packed = *src;
            unsigned int mask;
            switch (packed) {
            default:
                mask = packed >> 4;
                if (mask != 0 && mask != 15) {
                    *pixels &= sMaskAndBits[mask];
                    *pixels |= sMaskOrBits[mask];
                } else if (mask == 0) {
                    *pixels = 0;
                }
                mask = *src & 15;
                pixels++;
                if (mask != 0 && mask != 15) {
                    *pixels &= sMaskAndBits[mask];
                    *pixels |= sMaskOrBits[mask];
                } else if (mask == 0) {
                    *pixels = 0;
                }
                pixels++;
                break;
            case 255:
                pixels += 2;
                break;
            case 0:
                *pixels++ = 0;
                *pixels++ = 0;
                break;
            }
        }
        row += stride;
    }
}

unsigned char gUnusedBytes[4] = { 0, 0, 0, 0 }; /* 4 bytes; nothing reads or writes them */
int gClipFrameIndex = 0;
int gClipFrameCount;

/* Incremented by CountVSync from the VSyncCallback handler; src/media_player.c reads it as a plain int. */
volatile int gVSyncCount;

int gClipVramX;
int gClipVramY;
int gClipFramesDecoded;
int gClipDecodeVSyncs;
short gClipPeakRunLevelCount[2] = { 0, 0 };
short gClipRunLevelCount;
short gClipUnpackVSyncs;
short gClipMaskVSyncs;
short gClipUploadVSyncs;
short gClipDecodeVSyncsLow;

int DecodeClipFrame(struct ClipFrameInfo *info)
{
    struct {
        RECT rect;
        int width;
        int height;
        short lumaQuant;
        short chromaQuant;
    } work;
    struct ClipChunk *chunk;
    u_long *vlc;
    unsigned int runLevelCount;
    int height;

    chunk = (struct ClipChunk *)(gClipFrameData + gClipFrame->offset);
    work.width = chunk->width;
    height = chunk->height;
    work.height = height;
    info->x = gClipFrame->x;
    info->y = gClipFrame->y;
    vlc = ((struct ClipChunk *)(gClipFrameData + gClipFrame->offset))->bitstream;
    runLevelCount = *vlc;
    if (runLevelCount <= 0x3ffff && height < 360) {
        /* each holds the gVSyncCount at its stage's start, then the vsyncs the stage took */
        int unpackVSyncs;
        int maskVSyncs;
        int uploadVSyncs;
        int x;
        int columnWords;
        const int *soundCue;
        int value;

        work.rect.x = 320;
        work.rect.y = 0;
        work.rect.w = 320;
        work.rect.h = work.height;
        ClearImage(&work.rect, 0, 0, 0);

        /* The VSync callback advances this counter during decoding. */
        unpackVSyncs = gVSyncCount;
        {
            struct ClipChunk *header = (struct ClipChunk *)(gClipFrameData + gClipFrame->offset);
            work.lumaQuant = header->lumaQuant;
            gLumaQuant = work.lumaQuant;
            work.chromaQuant = header->chromaQuant;
            gChromaQuant = work.chromaQuant;
        }
        UnpackNitrBsFrame(vlc, sRunLevels);
        maskVSyncs = gVSyncCount;
        x = 0;
        DecDCTin(sRunLevels, 0);
        work.rect.w = 16;
        work.rect.y = gClipVramY;
        work.rect.h = work.height;
        columnWords = work.height << 3;
        DecompressClipMask();
        uploadVSyncs = gVSyncCount;
        DrawSync(0);
        for (; x < work.width; x += 16) {
            DecDCTout(sColumnPixels, columnWords);
            DecDCToutSync(0);
            ApplyClipMask((unsigned int *)sColumnPixels, x, work.width, work.height);
            work.rect.x = (short)gClipVramX + x;
            DrawSync(0);
            LoadImage(&work.rect, sColumnPixels);
        }
        DrawSync(0);
        unpackVSyncs = maskVSyncs - unpackVSyncs;
        maskVSyncs = uploadVSyncs - maskVSyncs;
        value = work.width;
        uploadVSyncs = gVSyncCount - uploadVSyncs;
        info->height = work.height;
        info->width = value;
        soundCue = &gClipFrame->soundCue;
        value = *soundCue;
        gClipFramesDecoded++;
        gClipDecodeVSyncs += unpackVSyncs + maskVSyncs + uploadVSyncs;
        info->soundCue = value;
        if (unpackVSyncs == 3 && (unsigned int)gClipPeakRunLevelCount[0] < runLevelCount)
            gClipPeakRunLevelCount[0] = runLevelCount;
        gClipRunLevelCount = runLevelCount;
        gClipUnpackVSyncs = unpackVSyncs;
        gClipMaskVSyncs = maskVSyncs;
        gClipUploadVSyncs = uploadVSyncs;
        gClipDecodeVSyncsLow = gClipDecodeVSyncs;
        return 1;
    }
    work.rect.x = 320;
    work.rect.w = 320;
    work.rect.y = 0;
    work.rect.h = 480;
    return ClearImage(&work.rect, 255, 255, 255);
}

unsigned short gRunLevelCodes[256] = {
    0x0001, 0x0002, 0x0401, 0x0003, 0x0004, 0x0801, 0x0005, 0x0006,
    0x0c01, 0x0402, 0x0007, 0x0008, 0x1001, 0x0009, 0x1401, 0x000a,
    0x000b, 0x0403, 0x1801, 0x000c, 0x000d, 0x1c01, 0x000e, 0x000f,
    0x0802, 0x2001, 0x0404, 0x0010, 0x0011, 0x2401, 0x0012, 0x0013,
    0x0405, 0x0014, 0x2801, 0x0015, 0x0c02, 0x3001, 0x0017, 0x0016,
    0x2c01, 0x0018, 0x001c, 0x0019, 0x0406, 0x0803, 0x001b, 0x001a,
    0x3401, 0x001d, 0x0407, 0x1002, 0x001f, 0x001e, 0x3801, 0x0020,
    0x0021, 0x0408, 0x0023, 0x0022, 0x1402, 0x0024, 0x0025, 0x0804,
    0x0409, 0x0418, 0x0026, 0x3c01, 0x0027, 0x0c03, 0x1c03, 0x0028,
    0x0029, 0x002a, 0x002b, 0x040a, 0x002c, 0x1802, 0x002d, 0x002f,
    0x002e, 0x4001, 0x0805, 0x0030, 0x040b, 0x0031, 0x0033, 0x0032,
    0x1c02, 0x0034, 0x1003, 0x0035, 0x4401, 0x040c, 0x0037, 0x0036,
    0x0038, 0x0039, 0x5401, 0x003a, 0x0c04, 0x040d, 0x5c01, 0x2002,
    0x003b, 0x0806, 0x4c01, 0x003c, 0x2402, 0x6001, 0x4801, 0x003d,
    0x040e, 0x5001, 0x003e, 0x0040, 0x003f, 0x1403, 0x0041, 0x6401,
    0x0042, 0x5801, 0x0043, 0x040f, 0x3402, 0x0044, 0x0045, 0x0807,
    0x0046, 0x0047, 0x1004, 0x0410, 0x0048, 0x0c05, 0x0049, 0x0411,
    0x004a, 0x1803, 0x004b, 0x3403, 0x2c02, 0x0808, 0x0413, 0x004c,
    0x2802, 0x6801, 0x004d, 0x004e, 0x0412, 0x004f, 0x0c06, 0x0417,
    0x0050, 0x0051, 0x1404, 0x0053, 0x0052, 0x2403, 0x0809, 0x0054,
    0x3002, 0x0414, 0x0055, 0x0056, 0x7801, 0x0057, 0x1005, 0x0058,
    0x2003, 0x0415, 0x0059, 0x1c04, 0x005a, 0x0419, 0x7001, 0x080a,
    0x005b, 0x005c, 0x6c01, 0x0416, 0x0c07, 0x0060, 0x1804, 0x0061,
    0x005e, 0x005d, 0x2404, 0x0063, 0x005f, 0x0064, 0x0062, 0x0065,
    0x080b, 0x1405, 0x7401, 0x0066, 0x0067, 0x041a, 0x0c08, 0x0068,
    0x3404, 0x0069, 0x006a, 0x2405, 0x041b, 0x3414, 0x006b, 0x1006,
    0x2004, 0x080c, 0x2406, 0x041c, 0x006c, 0x7c01, 0x2803, 0x3415,
    0x2408, 0x006d, 0x3802, 0x006e, 0x041d, 0x080d, 0x006f, 0x1805,
    0x041e, 0x0070, 0x2c03, 0x0c09, 0x8001, 0x1007, 0x0071, 0x3003,
    0x1c05, 0x1406, 0x3405, 0x0072, 0x080e, 0x1806, 0x0074, 0x041f,
    0x0077, 0x2409, 0x240a, 0x0073, 0x0075, 0x2407, 0x2005, 0x0076,
    0x007a, 0x0c0e, 0x0078, 0x3406, 0x007b, 0x0c0a, 0x080f, 0x0000,
};

void InitClipDecoder(int x, int y)
{
    gClipVramX = x;
    gClipVramY = y;
    DecDCTReset(0);
    BuildSignedRunLevelCodes(gRunLevelCodes);
    gVSyncCount = 0;
    gClipFramesDecoded = 0;
    gClipDecodeVSyncs = 0;
}

int OpenClip(unsigned char *data)
{
    int frameCount;

    data += 8;
    gClipFrameIndex = 0;
    frameCount = *(int *)data;
    data += sizeof(int);
    gClipFrame = (struct ClipFrame *)data;
    gClipFrameCount = frameCount;
    gClipFrameData = (unsigned char *)((struct ClipFrame *)data + frameCount);
    return 1;
}

int AdvanceClipFrame(void)
{
    int frame = gClipFrameIndex;
    if (frame < gClipFrameCount - 1) {
        gClipFrameIndex = frame + 1;
        gClipFrame++;
        return 1;
    }
    return 0;
}

int CountVSync(void)
{
    int count;

    count = gVSyncCount + 1;
    gVSyncCount = count;
    return count;
}

