/* Media player Play/Exit 3D cube: assembles the eight cube pieces (gModels[41..48]) on the frame in
 * gModels[40] and spins them. Also holds the text and label band positions and the ring's rest and
 * overshoot positions. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include "graphics.h"
#include "grid_effects.h"
#include "media_cube.h"
#include "media_player.h"
#include "text.h"
#include "media_cube_tmd.h"

extern u_long gPointTmd[2];
extern u_long gCubePieceTim;
extern u_long gCubePieceSelectedTim;

enum CubePieceKind {
    CUBE_PIECE_PLAY,
    CUBE_PIECE_EXIT
};

u_long *gCubePieceTmds[2] = { gPlayPieceTmd, gExitPieceTmd };
u_long *gCubePieceSelectedTmds[2] = { gPlayPieceSelectedTmd, gExitPieceSelectedTmd };

u_char gCubePieceKinds[8] = {
    CUBE_PIECE_PLAY, CUBE_PIECE_EXIT, CUBE_PIECE_EXIT, CUBE_PIECE_PLAY,
    CUBE_PIECE_PLAY, CUBE_PIECE_EXIT, CUBE_PIECE_EXIT, CUBE_PIECE_PLAY
};

SVECTOR gTextBandPosition = { 0, 200, 0, 0 };
SVECTOR gLabelBandPosition = { 0, -250, 0, 0 };
SVECTOR gRingRestPosition = { 0, 60, -300, 0 };
SVECTOR gRingRiseOvershoot = { 0, -40, -300, 0 };
SVECTOR gRingDropOvershoot = { 0, 160, -300, 0 };
SVECTOR gCubePosition = { -240, 140, -1034, 0 };
enum CubePhase {
    CUBE_ASSEMBLING,
    CUBE_ASSEMBLED
};
u_char gCubePhase;
int gExitPieceAngle;
int gPlayPieceAngle;
int gCubeAssemblyFrame;
SVECTOR gCubePieceSteps[8];

void InitMediaPlayerCube(void)
{
    unsigned int i;

    InitModelCoordinate(&gModels[40]);
    SetModelTmd(gPointTmd, &gModels[40]);
    LinkModelObjects(&gModels[40], &gModels[1].coord);
    gModels[40].coord.coord.t[0] = gCubePosition.vx;
    gModels[40].coord.coord.t[1] = gCubePosition.vy;
    gModels[40].coord.coord.t[2] = gCubePosition.vz - gView.vpz;
    UpdateModelMatrix(&gModels[40]);
    for (i = 0; i < 8; i++) {
        InitModelCoordinate(&gModels[i + 41]);
        SetModelTmd(gCubePieceTmds[gCubePieceKinds[i]], &gModels[i + 41]);
        LinkModelObjects(&gModels[i + 41], &gModels[40].coord);
        gModels[i + 41].coord.coord.t[0] = gCubePieceStarts[i].vx;
        gModels[i + 41].coord.coord.t[1] = gCubePieceStarts[i].vy;
        gModels[i + 41].coord.coord.t[2] = gCubePieceStarts[i].vz;
        gModels[i + 41].object.attribute = GsDOFF;
        UpdateModelMatrix(&gModels[i + 41]);
        gCubePieceSteps[i].vx = (gCubePieceStarts[i].vx - gCubeCorners[i].vx) / 10;
        gCubePieceSteps[i].vy = (gCubePieceStarts[i].vy - gCubeCorners[i].vy) / 10;
        gCubePieceSteps[i].vz = (gCubePieceStarts[i].vz - gCubeCorners[i].vz) / 10;
    }
    gExitPieceAngle = 0;
    gPlayPieceAngle = 0;
    gCubeAssemblyFrame = 0;
    gCubePhase = CUBE_ASSEMBLING;
    UploadTim(&gCubePieceTim);
    UploadTim(&gCubePieceSelectedTim);
    gFlatLights[2].vx = 0;
    gFlatLights[2].vy = 0;
    gFlatLights[2].vz = -700;
    gFlatLights[2].g = 0x30;
    gFlatLights[2].r = 0x30;
    GsSetFlatLight(2, &gFlatLights[2]);
}

void DrawMediaPlayerCube(short selection)
{
    unsigned int i;

    switch (gCubePhase) {
    case CUBE_ASSEMBLING: {
        if (gCubeAssemblyFrame < 10) {
            Model *base, *current;
            int deltaZ;
            Model *model;
            GsOT *ots;

            i = 0;
            ots = gOt;
            base = gModels;
            model = base + 41;
            do {
                current = (Model *)((41 + i)*sizeof(*base) + (u_long)base);
                current->coord.coord.t[0] -= gCubePieceSteps[i].vx;
                current->coord.coord.t[1] -= gCubePieceSteps[i].vy;
                deltaZ = gCubePieceSteps[i].vz;
                current->object.attribute &= ~GsDOFF;
                current->coord.coord.t[2] -= deltaZ;
                UpdateModelMatrix(model);
                SortModel(model, ots[gActiveBuffer]);
                model++;
            } while (++i < 8);
            gCubeAssemblyFrame++;
        } else {
            Model *model;
            GsOT *ots;

            i = 0;
            ots = gOt;
            model = &gModels[41];
            do {
                SortModel(model++, ots[gActiveBuffer]);
            } while (++i < 8);
            gCubePhase = CUBE_ASSEMBLED;
        }
        break;
    }
    case CUBE_ASSEMBLED: {
        Model *base;
        int selectedKind;
        int kind;

        if (selection < PLAYER_SELECT_WORD && gCaptionMoving == 0) {
            if (selection == PLAYER_SELECT_EXIT) {
                selectedKind = CUBE_PIECE_EXIT;
            } else if (selection == PLAYER_SELECT_PLAY) {
                selectedKind = CUBE_PIECE_PLAY;
            }
        } else {
            gExitPieceAngle = 0;
            gPlayPieceAngle = 0;
        }
        base = &gModels[41];
        for (i = 0; i < 8; i++) {
            InitModelCoordinate(&gModels[i + 41]);
            kind = gCubePieceKinds[i];
            if (kind == selectedKind) {
                SetModelTmd(gCubePieceSelectedTmds[kind], &gModels[i + 41]);
                if (i < 2) {
                    if (i == 1) {
                        gExitPieceAngle += 45;
                        gPlayPieceAngle = 0;
                        base[1].rot.vy = gExitPieceAngle;
                    } else if (i == 0) {
                        gPlayPieceAngle += 45;
                        gExitPieceAngle = 0;
                        base[0].rot.vy = gPlayPieceAngle;
                    }
                } else {
                    gModels[i + 41].rot.vy = 0;
                }
            } else {
                SetModelTmd(gCubePieceTmds[kind], &gModels[i + 41]);
                gModels[i + 41].rot.vy = 0;
            }
            LinkModelObjects(&gModels[i + 41], &base[-1].coord);
            gModels[i + 41].coord.coord.t[0] = gCubeCorners[i].vx;
            gModels[i + 41].coord.coord.t[1] = gCubeCorners[i].vy;
            gModels[i + 41].coord.coord.t[2] = gCubeCorners[i].vz;
            UpdateModelMatrix(&gModels[i + 41]);
            gModels[i + 41].object.attribute &= ~GsDOFF;
            SortModel(&gModels[i + 41], gOt[gActiveBuffer]);
        }
        break;
    }
    }
}
