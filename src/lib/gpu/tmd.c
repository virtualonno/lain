#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <memory.h>
#include <stdio.h>

#define TIM_ID 0x10
#define TIM_HAS_CLUT 0x08
#define TMD_SEMITRANS 0x02000000
#define TMD_PACKET(code, flags, in, out) \
    ((u_long)(code) << 24 | (u_long)(flags) << 16 | (in) << 8 | (out))

/* A TMD object-table entry as stored: vert, norm and prim are byte offsets from the table's start. */
typedef struct {
    u_long vert, nvert;
    u_long norm, nnorm;
    u_long prim, nprim;
    u_long scale;
} TmdFileObject;

typedef struct {
    u_long id, flags, nobj;
    TmdFileObject object[1];
} TmdFile;

static u_long *tim;
static SVECTOR *vertices;
static SVECTOR *normals;
static u_char *primitive;
static int primitiveCount;

static int readTim(u_long *addr, TIM_IMAGE *image);
static int readObject(TmdFile *file, int index, u_char **prim, SVECTOR **vert, SVECTOR **norm);
static int readPrimitive(u_char *packet, TMD_PRIM *p);

int OpenTIM(u_long *addr)
{
    tim = addr;
    return 0;
}

TIM_IMAGE *ReadTIM(TIM_IMAGE *image)
{
    int words = readTim(tim, image);

    if (words == -1)
        return 0;
    tim += words;
    return image;
}

int OpenTMD(u_long *tmd, int obj_no)
{
    return primitiveCount = readObject((TmdFile *)tmd, obj_no, &primitive, &vertices, &normals);
}

#define COPY_VECTOR(dst, table, i) \
    (dst).vx = (table)[i].vx; \
    (dst).vy = (table)[i].vy; \
    (dst).vz = (table)[i].vz

TMD_PRIM *ReadTMD(TMD_PRIM *p)
{
    int size = readPrimitive(primitive, p);
    SVECTOR *vert, *norm;

    if (size < 0)
        return 0;
    p->v_ofs = vert = vertices;
    p->n_ofs = norm = normals;
    COPY_VECTOR(p->n0, norm, p->norm0);
    COPY_VECTOR(p->n1, norm, p->norm1);
    COPY_VECTOR(p->n2, norm, p->norm2);
    primitive += size;
    COPY_VECTOR(p->n3, norm, p->norm3);
    COPY_VECTOR(p->x0, vert, p->vert0);
    COPY_VECTOR(p->x1, vert, p->vert1);
    COPY_VECTOR(p->x2, vert, p->vert2);
    COPY_VECTOR(p->x3, vert, p->vert3);
    return p;
}

static int readTim(u_long *addr, TIM_IMAGE *image)
{
    u_long id = *addr++;
    int clutWords;
    int pixelWords;

    if (id != TIM_ID)
        return -1;
    image->mode = *addr++;
    if (GetGraphDebug() == 2)
        printf("id  =%08x\n", id);
    if (GetGraphDebug() == 2)
        printf("mode=%08x\n", image->mode);
    if (GetGraphDebug() == 2)
        printf("timaddr=%08x\n", addr);
    if (image->mode & TIM_HAS_CLUT) {
        clutWords = *addr / sizeof(u_long);
        image->crect = (RECT *)(addr + 1);
        image->caddr = addr + 3;
        addr += clutWords;
    } else {
        clutWords = 0;
        image->crect = 0;
        image->caddr = 0;
    }
    pixelWords = *addr / sizeof(u_long);
    image->prect = (RECT *)(addr + 1);
    image->paddr = addr + 3;
    return 2 + clutWords + pixelWords;
}

static int readObject(TmdFile *file, int index, u_char **prim, SVECTOR **vert, SVECTOR **norm)
{
    TmdFileObject *object = file->object;
    u_char *base = (u_char *)object;

    if (GetGraphDebug() == 2)
        printf("analizing TMD...\n");
    if (GetGraphDebug() == 2)
        printf("\tid=%08X, flags=%d, nobj=%d, objid=%d\n", file->id, file->flags, file->nobj, index);
    if (GetGraphDebug() == 2)
        printf("\tvert=%08X, nvert=%d\n", object[index].vert, object[index].nvert);
    if (GetGraphDebug() == 2)
        printf("\tnorm=%08X, nnorm=%d\n", object[index].norm, object[index].nnorm);
    if (GetGraphDebug() == 2)
        printf("\tprim=%08X, nprim=%d\n", object[index].prim, object[index].nprim);
    *vert = (SVECTOR *)(base + object[index].vert);
    *norm = (SVECTOR *)(base + object[index].norm);
    *prim = base + object[index].prim;
    return object[index].nprim;
}

#define COPY_COLOR(dst, src) \
    p->r##dst = q->r##src; \
    p->g##dst = q->g##src; \
    p->b##dst = q->b##src
#define COPY_UV(i) p->u##i = q->tu##i; p->v##i = q->tv##i
#define COPY_TEX3() \
    p->tpage = q->tpage; p->clut = q->clut; \
    COPY_UV(0); COPY_UV(1); COPY_UV(2)
#define COPY_TEX4() COPY_TEX3(); COPY_UV(3)
#define COPY_VERTS3() p->vert0 = q->v0; p->vert1 = q->v1; p->vert2 = q->v2
#define COPY_VERTS4() COPY_VERTS3(); p->vert3 = q->v3
#define COPY_NORMS3(n1, n2) p->norm0 = q->n0; p->norm1 = q->n##n1; p->norm2 = q->n##n2
#define COPY_NORMS4(n1, n2, n3) COPY_NORMS3(n1, n2); p->norm3 = q->n##n3
#define PRINT_TYPE(name) if (GetGraphDebug() == 2) printf(name)

static int readPrimitive(u_char *packet, TMD_PRIM *p)
{
    memset(p, 0, sizeof(*p));
    p->id = *(u_long *)packet;
    switch (p->id & ~TMD_SEMITRANS) {
    case TMD_PACKET(GPU_COM_F3, 0, 3, 4):
    case TMD_PACKET(GPU_COM_F3, 2, 3, 4): {
        TMD_P_F3 *q = (TMD_P_F3 *)packet;
        PRINT_TYPE("F3L ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0);
        COPY_VERTS3();
        COPY_NORMS3(0, 0);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_G3, 0, 4, 6):
    case TMD_PACKET(GPU_COM_G3, 2, 4, 6): {
        TMD_P_G3 *q = (TMD_P_G3 *)packet;
        PRINT_TYPE("G3L ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0);
        COPY_VERTS3();
        COPY_NORMS3(1, 2);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_TF3, 0, 5, 7):
    case TMD_PACKET(GPU_COM_TF3, 2, 5, 7): {
        TMD_P_TF3 *q = (TMD_P_TF3 *)packet;
        PRINT_TYPE("FT3L ");
        COPY_TEX3();
        COPY_VERTS3();
        COPY_NORMS3(0, 0);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_TG3, 0, 6, 9):
    case TMD_PACKET(GPU_COM_TG3, 2, 6, 9): {
        TMD_P_TG3 *q = (TMD_P_TG3 *)packet;
        PRINT_TYPE("GT3L ");
        COPY_TEX3();
        COPY_VERTS3();
        COPY_NORMS3(1, 2);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NF3, 1, 3, 4):
    case TMD_PACKET(GPU_COM_NF3, 3, 3, 4): {
        TMD_P_NF3 *q = (TMD_P_NF3 *)packet;
        PRINT_TYPE("F3 ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0);
        COPY_VERTS3();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NG3, 1, 5, 6):
    case TMD_PACKET(GPU_COM_NG3, 3, 5, 6): {
        TMD_P_NG3 *q = (TMD_P_NG3 *)packet;
        PRINT_TYPE("G3 ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 1); COPY_COLOR(2, 2);
        COPY_VERTS3();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NTF3, 1, 6, 7):
    case TMD_PACKET(GPU_COM_NTF3, 3, 6, 7): {
        TMD_P_TNF3 *q = (TMD_P_TNF3 *)packet;
        PRINT_TYPE("FT3 ");
        COPY_TEX3();
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0);
        COPY_VERTS3();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NTG3, 1, 8, 9):
    case TMD_PACKET(GPU_COM_NTG3, 3, 8, 9): {
        TMD_P_TNG3 *q = (TMD_P_TNG3 *)packet;
        PRINT_TYPE("GT3 ");
        COPY_TEX3();
        COPY_VERTS3();
        COPY_COLOR(0, 0); COPY_COLOR(1, 1); COPY_COLOR(2, 2);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_F4, 0, 4, 5):
    case TMD_PACKET(GPU_COM_F4, 2, 4, 5): {
        TMD_P_F4 *q = (TMD_P_F4 *)packet;
        PRINT_TYPE("F4L ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0); COPY_COLOR(3, 0);
        COPY_VERTS4();
        COPY_NORMS4(0, 0, 0);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_G4, 0, 5, 8):
    case TMD_PACKET(GPU_COM_G4, 2, 5, 8): {
        TMD_P_G4 *q = (TMD_P_G4 *)packet;
        PRINT_TYPE("G4L ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0); COPY_COLOR(3, 0);
        COPY_VERTS4();
        COPY_NORMS4(1, 2, 3);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_TF4, 0, 7, 9):
    case TMD_PACKET(GPU_COM_TF4, 2, 7, 9): {
        TMD_P_TF4 *q = (TMD_P_TF4 *)packet;
        PRINT_TYPE("FT4L ");
        COPY_TEX4();
        COPY_VERTS4();
        COPY_NORMS4(0, 0, 0);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_TG4, 0, 8, 12):
    case TMD_PACKET(GPU_COM_TG4, 2, 8, 12): {
        TMD_P_TG4 *q = (TMD_P_TG4 *)packet;
        PRINT_TYPE("GT4L ");
        COPY_TEX4();
        COPY_VERTS4();
        COPY_NORMS4(1, 2, 3);
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NF4, 1, 3, 5):
    case TMD_PACKET(GPU_COM_NF4, 3, 3, 5): {
        TMD_P_NF4 *q = (TMD_P_NF4 *)packet;
        PRINT_TYPE("F4 ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0); COPY_COLOR(3, 0);
        COPY_VERTS4();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NG4, 1, 6, 8):
    case TMD_PACKET(GPU_COM_NG4, 3, 6, 8): {
        TMD_P_NG4 *q = (TMD_P_NG4 *)packet;
        PRINT_TYPE("G4 ");
        COPY_COLOR(0, 0); COPY_COLOR(1, 1); COPY_COLOR(2, 2); COPY_COLOR(3, 3);
        COPY_VERTS4();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NTF4, 1, 7, 9):
    case TMD_PACKET(GPU_COM_NTF4, 3, 7, 9): {
        TMD_P_TNF4 *q = (TMD_P_TNF4 *)packet;
        PRINT_TYPE("FT4 ");
        COPY_TEX4();
        COPY_COLOR(0, 0); COPY_COLOR(1, 0); COPY_COLOR(2, 0); COPY_COLOR(3, 0);
        COPY_VERTS4();
        return sizeof(*q);
    }
    case TMD_PACKET(GPU_COM_NTG4, 1, 10, 12):
    case TMD_PACKET(GPU_COM_NTG4, 3, 10, 12): {
        TMD_P_TNG4 *q = (TMD_P_TNG4 *)packet;
        PRINT_TYPE("GT4 ");
        COPY_TEX4();
        COPY_VERTS4();
        COPY_COLOR(0, 0); COPY_COLOR(1, 1); COPY_COLOR(2, 2); COPY_COLOR(3, 3);
        return sizeof(*q);
    }
    default:
        printf("unsupported type (%08x)\n", p->id & ~TMD_SEMITRANS);
        return -1;
    }
}
