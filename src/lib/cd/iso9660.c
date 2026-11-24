/*
 * CdSearchFile: finds a file by its path on an ISO 9660 disc. The little-endian path table
 * is read on the first search and again once the lid has been opened (CD_nopen); the
 * records of the last directory searched stay cached.
 */
#include <stdio.h>
#include <string.h>
#include "cd_int.h"

#define CD_SECTOR_SIZE 2048
#define ISO_PVD_SECTOR 16
#define ISO_ROOT_DIR 1	/* the path table's first record */
#define NAME_COMPARE_LEN 12

/* Byte offsets: in the primary volume descriptor, in a path table record, in a directory record */
#define ISO_PVD_PATH_TABLE_L	140	/* sector of the little-endian path table */
#define ISO_PT_NAME_LEN		0
#define ISO_PT_EXTENT		2
#define ISO_PT_PARENT		6	/* low byte of the parent's record number */
#define ISO_PT_NAME		8
#define ISO_DR_LEN		0
#define ISO_DR_EXTENT		2
#define ISO_DR_SIZE		10
#define ISO_DR_NAME_LEN		32
#define ISO_DR_NAME		33

/* A directory read from the path table: id is its 1-based record number, parent the parent's id */
typedef struct {
    int id;
    int parent;
    int sector;
    char name[32];
} IsoPath;

static int cachedDir = 0;
static int mediaOpenCount = -1;
static CdlFILE files[CdlMAXFILE];
static IsoPath paths[CdlMAXDIR];
static unsigned char sectorBuf[CD_SECTOR_SIZE];

static int CD_matchfile(char *entry, char *name);
static int CD_newmedia(void);
static int CD_finddir(int parent, char *name);
static int CD_cachefile(int dir);
static int cd_read(int sectors, int sector, unsigned char *buffer);

CdlFILE *CdSearchFile(CdlFILE *fp, char *name)
{
    char component[32];
    char *src, *dst;
    int dir, depth;

    if (mediaOpenCount != CD_nopen) {
        if (!CD_newmedia())
            return NULL;
        mediaOpenCount = CD_nopen;
    }
    if (*name != '\\')
        return NULL;

    component[0] = '\0';
    dir = ISO_ROOT_DIR;
    src = name;
    depth = 0;
    do {
        dst = component;
        while (*src != '\\' && *src != '\0') {
            *dst++ = *src++;
        }
        if (*src == '\0')
            break;
        ++src;
        *dst = '\0';
        dir = CD_finddir(dir, component);
        if (dir == -1) {
            component[0] = '\0';
            break;
        }
    } while (++depth < CdlMAXLEVEL);

    if (depth >= CdlMAXLEVEL) {
        if (CD_debug > 0)
            printf("%s: path level (%d) error\n", name, depth);
        return NULL;
    }
    if (!component[0]) {
        if (CD_debug > 0)
            printf("%s: dir was not found\n", name);
        return NULL;
    }
    *dst = '\0';
    if (!CD_cachefile(dir)) {
        if (CD_debug > 0)
            printf("CdSearchFile: disc error\n");
        return NULL;
    }
    if (CD_debug >= 2)
        printf("CdSearchFile: searching %s...\n", component);
    for (depth = 0; depth < CdlMAXFILE; depth++) {
        if (!files[depth].name[0])
            break;
        if (CD_matchfile(files[depth].name, component)) {
            if (CD_debug >= 2)
                printf("%s:  found\n", component);
            *fp = files[depth];
            return &files[depth];
        }
    }
    if (CD_debug > 0)
        printf("%s: not found\n", component);
    return NULL;
}

static int CD_matchfile(char *entry, char *name)
{
    return strncmp(entry, name, NAME_COMPARE_LEN) == 0;
}

static int CD_newmedia(void)
{
    unsigned char *record;
    int tableSector;
    int i;

    if (cd_read(1, ISO_PVD_SECTOR, sectorBuf) != 1) {
        if (CD_debug > 0)
            printf("CD_newmedia: Read error in cd_read(PVD)\n");
        return 0;
    }
    if (strncmp((char *)sectorBuf + 1, "CD001", 5)) {
        if (CD_debug > 0)
            printf("CD_newmedia: Disc format error in cd_read(PVD)\n");
        return 0;
    }
    memcpy(&tableSector, sectorBuf + ISO_PVD_PATH_TABLE_L, sizeof(tableSector));
    if (cd_read(1, tableSector, sectorBuf) != 1) {
        if (CD_debug > 0)
            printf("CD_newmedia: Read error (PT:%08x)\n", tableSector);
        return 0;
    }
    if (CD_debug >= 2)
        printf("CD_newmedia: sarching dir..\n");

    record = sectorBuf;
    for (i = 0; record < sectorBuf + sizeof(sectorBuf); ) {
        if (!record[ISO_PT_NAME_LEN])
            break;
        memcpy(&paths[i].sector, record + ISO_PT_EXTENT, sizeof(paths[i].sector));
        paths[i].parent = record[ISO_PT_PARENT];
        paths[i].id = i + 1;
        memcpy(paths[i].name, record + ISO_PT_NAME, record[ISO_PT_NAME_LEN]);
        paths[i].name[record[ISO_PT_NAME_LEN]] = '\0';
        record += ISO_PT_NAME + record[ISO_PT_NAME_LEN] + (record[ISO_PT_NAME_LEN] & 1);
        if (CD_debug >= 2)
            printf("\t%08x,%04x,%04x,%s\n", paths[i].sector, paths[i].id,
                   paths[i].parent, paths[i].name);
        if (++i >= CdlMAXDIR)
            break;
    }
    if (i < CdlMAXDIR)
        paths[i].parent = 0;
    cachedDir = 0;
    if (CD_debug >= 2)
        printf("CD_newmedia: %d dir entries found\n", i);
    return 1;
}

static int CD_finddir(int parent, char *name)
{
    int i;

    for (i = 0; i < CdlMAXDIR; i++) {
        if (!paths[i].parent)
            break;
        if (paths[i].parent == parent && !strcmp(name, paths[i].name))
            return i + 1;
    }
    return -1;
}

static int CD_cachefile(int dir)
{
    unsigned char *record;
    int extent;
    int i;

    if (dir == cachedDir)
        return 1;
    if (cd_read(1, (paths + dir - 1)->sector, sectorBuf) != 1) {
        if (CD_debug > 0)
            printf("CD_cachefile: dir not found\n");
        return -1;
    }
    if (CD_debug >= 2)
        printf("CD_cachefile: searching...\n");

    record = sectorBuf;
    for (i = 0; record < sectorBuf + sizeof(sectorBuf); ) {
        if (!record[ISO_DR_LEN])
            break;
        memcpy(&extent, record + ISO_DR_EXTENT, sizeof(extent));
        CdIntToPos(extent, &files[i].pos);
        memcpy(&files[i].size, record + ISO_DR_SIZE, sizeof(files[i].size));
        switch (i) {
        case 0:
            memcpy(files[0].name, ".", 2);
            break;
        case 1:
            memcpy(files[1].name, "..", 3);
            break;
        default:
            memcpy(files[i].name, record + ISO_DR_NAME, record[ISO_DR_NAME_LEN]);
            files[i].name[record[ISO_DR_NAME_LEN]] = '\0';
        }
        if (CD_debug >= 2)
            printf("\t(%02x:%02x:%02x) %8d %s\n", files[i].pos.minute,
                   files[i].pos.second, files[i].pos.sector, files[i].size, files[i].name);
        record += record[ISO_DR_LEN];
        if (++i >= CdlMAXFILE)
            break;
    }
    cachedDir = dir;
    if (i < CdlMAXFILE)
        files[i].name[0] = '\0';
    if (CD_debug >= 2)
        printf("CD_cachefile: %d files found\n", i);
    return 1;
}

static int cd_read(int sectors, int sector, unsigned char *buffer)
{
    CdlLOC pos;

    CdIntToPos(sector, &pos);
    CdControl(CdlSetloc, (u_char *)&pos, NULL);
    CdRead(sectors, (u_long *)buffer, CdlModeSpeed);
    return CdReadSync(0, NULL) == 0;
}
