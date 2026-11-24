#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>

typedef union {
    u_long word;
    struct {
        u_short runLength;
        u_char flags;
        u_char mode;
    } header;
} TmdPrimitiveWord;

void GsLinkObject4(u_long base, GsDOBJ2 *object, int index)
{
    u_char mode = 0;
    u_char flags = 0;
    u_long i;
    u_long runLength = 0;
    struct TMD_STRUCT *tmd = &((struct TMD_STRUCT *)base)[index];
    TmdPrimitiveWord *primitive, *run;
    u_long count;

    object->tmd = (u_long *)tmd;
    primitive = (TmdPrimitiveWord *)tmd->primtop;
    count = tmd->primn;
    run = primitive;
    for (i = 0; i < count; i++) {
        u_char previousMode = mode;
        u_char previousFlags = flags;
        u_long header = primitive->word;

        mode = header >> 24;
        flags = header >> 16;
        if (previousMode && (mode != previousMode || flags != previousFlags)) {
            run->header.runLength = runLength;
            runLength = 0;
            run = primitive;
        }
        /* the GPU code without its semitransparency bit */
        switch (mode & 0xfd) {
        case 0x20:
            if (flags & 4)
                primitive += 6;
            else
                primitive += 4;
            break;
        case 0x24:
        case 0x31:
            primitive += 6;
            break;
        case 0x30:
            if (flags & 4)
                primitive += 7;
            else
                primitive += 5;
            break;
        case 0x25:
        case 0x34:
        case 0x39:
            primitive += 7;
            break;
        case 0x28:
            if (flags & 4)
                primitive += 8;
            else
                primitive += 5;
            break;
        case 0x2c:
        case 0x2d:
            primitive += 8;
            break;
        case 0x21:
        case 0x29:
            primitive += 4;
            break;
        case 0x3d:
            primitive += 11;
            break;
        case 0x38:
            if (flags & 4)
                primitive += 9;
            else
                primitive += 6;
            break;
        case 0x35:
        case 0x3c:
            primitive += 9;
            break;
        default:
            printf("GPU CODE %02xH not assigned.\n", mode);
            break;
        }
        runLength++;
    }
    run->header.runLength = runLength;
}
