#include <sys/types.h>
#include <libpress.h>
#include <stdlib.h>

#define SAMPLES_PER_BLOCK 28
#define WORDS_PER_BLOCK 8
#define PCM_LIMIT 0x7800
#define SPU_FLAG_END 0x0100
#define SPU_FLAG_REPEAT 0x0200
#define SPU_FLAG_LOOP_START 0x0400

static short saved_shift = -1;

static void quantize_block(short *src, short *dest, short *shift, int byte_swap, long *scaled)
{
    long peak = 0;
    int i = 0;
    long level;
    int mask;

    do {
        u_short raw = *src;
        short bits;
        int pcm;
        long magnitude;
        long value;

        if (byte_swap)
            raw = (raw >> 8) | ((raw & 0xff) << 8);
        bits = raw;
        pcm = bits;
        if (pcm >= PCM_LIMIT)
            bits = PCM_LIMIT - 1;
        if (pcm < -PCM_LIMIT)
            bits = -PCM_LIMIT;
        value = bits < 0 ? -((-bits) << 10) : (bits << 10);
        scaled[i] = value;
        magnitude = labs(value);
        if (peak < magnitude)
            peak = magnitude;
        i++;
        src++;
    } while (i < SAMPLES_PER_BLOCK);

    level = peak < 0 ? -((-peak) >> 10) : (peak >> 10);
    if (level > 32767)
        level = 32767;
    if (level < -32768)
        level = -32768;

    mask = 0x4000;
    for (*shift = 0; *shift < 12; (*shift)++) {
        if ((level + (mask >> 3)) & mask)
            break;
        mask >>= 1;
    }

    i = 0;
    do {
        long value = scaled[i] << *shift;
        value = value < 0 ? -((-value) >> 10) : (value >> 10);
        value = (value + 0x800) & ~0xfff;
        if (value > 32767)
            value = 32767;
        if (value < -32768)
            value = -32768;
        *dest++ = value;
        i++;
    } while (i < SAMPLES_PER_BLOCK);
}

static long encode(short *src, short *dest, long samples, long loop_start,
    int loop, int byte_swap, int proceed, short *work)
{
    short quantized[SAMPLES_PER_BLOCK];
    long scaled[SAMPLES_PER_BLOCK];
    short shift = 0;
    int words = 0;
    int end = 0;
    short *block = quantized;
    long *scaled_block = scaled;
    int i;

    if (work) {
        block = work;
        scaled_block = (long *)(block + SAMPLES_PER_BLOCK);
    }

    switch (proceed) {
    case ENCSPU_ENCODE_WHOLE:
    case ENCSPU_ENCODE_START:
        for (i = WORDS_PER_BLOCK - 1; i >= 0; i--)
            dest[words++] = 0;
        break;
    case ENCSPU_ENCODE_CONTINUE:
    case ENCSPU_ENCODE_END:
        if (saved_shift < 0)
            return ENCSPU_ENCODE_ERROR;
        shift = saved_shift;
        break;
    default:
        return ENCSPU_ENCODE_ERROR;
    }

    i = 0;
    do {
        u_short flags;
        short *samp;
        int j;

        quantize_block(src + i, block, &shift, byte_swap, scaled_block);
        flags = SPU_FLAG_REPEAT;
        if (loop && loop_start >= 0 && i >= loop_start && i < loop_start + SAMPLES_PER_BLOCK)
            flags |= SPU_FLAG_LOOP_START;
        if ((proceed == ENCSPU_ENCODE_WHOLE || proceed == ENCSPU_ENCODE_END) &&
            i >= samples - SAMPLES_PER_BLOCK) {
            end = 1;
            if (loop)
                flags |= SPU_FLAG_END;
        }

        dest[words++] = flags | shift;
        samp = block;
        for (j = 0; j < SAMPLES_PER_BLOCK; j += 4)
            dest[words++] = (samp[j + 3] & 0xf000) | (((u_short)samp[j + 2] >> 4) & 0x0f00) |
                (((u_short)samp[j + 1] >> 8) & 0x00f0) | ((u_short)samp[j] >> 12);
        i += SAMPLES_PER_BLOCK;
    } while (i < samples && !end);

    if (proceed & (ENCSPU_ENCODE_START | ENCSPU_ENCODE_CONTINUE)) {
        saved_shift = shift;
    } else {
        if (!loop) {
            dest[words++] = SPU_FLAG_LOOP_START | SPU_FLAG_REPEAT | SPU_FLAG_END;
            for (i = 1; i < WORDS_PER_BLOCK; i++)
                dest[words++] = 0;
        }
        saved_shift = -1;
    }
    return words * sizeof(short);
}

long EncSPU(ENCSPUENV *env)
{
    return encode(env->src, env->dest, env->size >> 1, env->loop_start >> 1,
        (signed char)env->loop, (signed char)env->byte_swap,
        (signed char)env->proceed, env->work);
}
