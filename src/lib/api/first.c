#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>
#include <strings.h>
#include <fs.h>

struct FileState {
    int mode;
};

typedef int (*FirstFile)();

#define DEVICE_TABLE (*(struct device_table **)0x150)
#define DEVICE_TABLE_BYTES (*(unsigned long *)0x154)

extern struct DIRENTRY *firstfile2(char *path, struct DIRENTRY *entry);

static struct {
    FirstFile saved;
    unsigned long unused; /* 4 bytes; nothing reads it */
    char deviceName[40];
} sFirst;

static struct DIRENTRY *firstFileHook(struct FileState *file, char *path,
                                     struct DIRENTRY *entry);

static inline int saveFirstFile(void)
{
    struct device_table *device = DEVICE_TABLE;
    struct device_table *end = device + DEVICE_TABLE_BYTES / sizeof(*device);

    if (device < end) {
        struct device_table *limit = end;

        do {
            if (device->dt_string && strcmp(device->dt_string, sFirst.deviceName) == 0) {
                sFirst.saved = device->dt_firstfile;
                return 1;
            }
        } while (++device < limit);
    }
    return 0;
}

static inline void setFirstFile(FirstFile callback)
{
    struct device_table *device = DEVICE_TABLE;
    struct device_table *end = device + DEVICE_TABLE_BYTES / sizeof(*device);

    if (device < end) {
        struct device_table *limit = end;

        do {
            if (device->dt_string && strcmp(device->dt_string, sFirst.deviceName) == 0) {
                device->dt_firstfile = callback;
                break;
            }
        } while (++device < limit);
    }
}

struct DIRENTRY *firstfile(char *path, struct DIRENTRY *entry)
{
    char *src = path;
    int next = *(signed char *)src;
    char *dst = sFirst.deviceName;

    while (next > ':') {
        *dst++ = *src++;
        next = *(signed char *)src;
    }
    *dst = '\0';
    if (!saveFirstFile()) {
        return 0;
    }

    /* The hook stands in for the device's firstfile until the kernel's search calls it. */
    setFirstFile((FirstFile)firstFileHook);
    return firstfile2(path, entry);
}

static struct DIRENTRY *firstFileHook(struct FileState *file, char *path,
                                     struct DIRENTRY *entry)
{
    struct FileState *state = file;
    char *filePath = path;
    struct DIRENTRY *dirEntry = entry;

    if (!state->mode) {
        state->mode = 1;
    }
    setFirstFile(sFirst.saved);
    return (struct DIRENTRY *)sFirst.saved(state, filePath, dirEntry);
}
