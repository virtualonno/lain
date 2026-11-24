/* Voice screen, run for tracks >= 518 and for track 733 after the end roll: speaks the player's name on
 * SPU voice 23 (tracks < 639, and 733), plays the XA track, and animates three tiles of the track image. */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include <libcd.h>
#include <libpress.h>
#include <libspu.h>
#include "special_scenes.h"
#include "loading.h"
#include "voice_screen.h"
#include "cd.h"
#include "graphics.h"
#include "grid_menu.h"
#include "grid_scene.h"
#include "heap.h"
#include "file_parts.h"
#include "kana_entry.h"
#include "lapks_player.h"
#include "media_player.h"
#include "pad.h"
#include "speech.h"

u_char gSpeechTileSequence[7] = { 0, 0, 2, 2, 1, 1, 0 };
u_char gHasEndRollSpheres = 0;
int gSpeechAdpcmSize;
u_char gSpeechClipCount;
enum VoiceScreenState {
    VOICE_FIRST_CLIP,
    VOICE_SPEAK_BEFORE,
    VOICE_PLAY_TRACK,
    VOICE_SPEAK_AFTER,
    VOICE_LAST_CLIP,
    VOICE_DONE
};
short gVoiceScreenState;
u_long gSpeechSpuAddr;
u_char gSpeechFrameCount;
void *gTimBuffer;
RECT gVramRect;

static inline int RequestSitePartRead(int part, void *buffer)
{
    u_short files[2] = { 6, 6 };
    FilePart *siteParts[2] = { gSiteAParts, gSiteBParts };

    gTimBuffer = buffer;
    return RequestFilePartRead(files[gCurrentDisc], part, siteParts[gCurrentDisc], buffer);
}

static inline void UploadTimBuffer(int x, int y)
{
    RECT rect;
    GsIMAGE tim;

    GsGetTimInfo((u_long *)gTimBuffer + 1, &tim);
    setRECT(&rect, x, y, tim.pw, tim.ph);
    LoadImage(&rect, tim.pixel);
    DrawSync(0);
    FreeHeapBlock(gTimBuffer);
}

void RunVoiceScreen(void)
{
    ENCSPUENV env;
    short encWork[84];
    SpuVoiceAttr voice;
    u_short timParts[5][2] = {
        {786, 551}, {789, 555}, {788, 553}, {19, 13}, {785, 550}
    };
    int pcmSize;
    short *pcm;
    short *adpcm;
    int request;

    gHasEndRollSpheres = 0;
    if (gTrackIndex == 733) {
        InitEndRollSpheres();
        gHasEndRollSpheres = 1;
    }

    if (gTrackIndex < 639 || gHasEndRollSpheres == 1) {
        pcm = BuildSpeechPcm(gPlayerName, &pcmSize, &gSpeechClipCount);
        gSpeechFrameCount = gSpeechClipCount * 18;
        if (gSpeechClipCount >= 6) {
            gSpeechClipCount = 6;
        }
        adpcm = (short *)AllocHeapBlock(pcmSize);
        if (adpcm == 0) {
            return;
        }
        env.src = pcm;
        env.dest = adpcm;
        env.loop = ENCSPU_ENCODE_NO_LOOP;
        env.byte_swap = ENCSPU_ENCODE_ENDIAN_LITTLE;
        env.proceed = ENCSPU_ENCODE_WHOLE;
        env.work = encWork;
        env.size = pcmSize;
        if ((gSpeechAdpcmSize = EncSPU(&env)) == ENCSPU_ENCODE_ERROR) {
            return;
        }
        FreeHeapBlock(pcm);
        SpuSetTransferStartAddr(gSpeechSpuAddr = SpuMalloc(gSpeechAdpcmSize));
        SpuWrite((u_char *)adpcm, gSpeechAdpcmSize);
        FreeHeapBlock(adpcm);

        voice.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA |
                     SPU_VOICE_ADSR_AMODE | SPU_VOICE_ADSR_SMODE | SPU_VOICE_ADSR_RMODE |
                     SPU_VOICE_ADSR_AR | SPU_VOICE_ADSR_DR | SPU_VOICE_ADSR_SR |
                     SPU_VOICE_ADSR_RR | SPU_VOICE_ADSR_SL;
        voice.voice = SPU_23CH;
        voice.volume.left = 13000;
        voice.volume.right = 13000;
        voice.pitch = 735;
        voice.a_mode = SPU_VOICE_LINEARIncN;
        voice.s_mode = SPU_VOICE_LINEARIncN;
        voice.r_mode = SPU_VOICE_LINEARDecN;
        voice.ar = 0;
        voice.dr = 0;
        voice.sr = 0;
        voice.rr = 0;
        voice.sl = 15;
        voice.addr = gSpeechSpuAddr;
        SpuSetVoiceAttr(&voice);
    }

    gVramRect.x = 0;
    gVramRect.y = 0;
    gVramRect.w = 640;
    gVramRect.h = 480;
    ClearImage(&gVramRect, 0, 0, 0);
    DrawSync(0);

    gVramRect.x = 320;
    gVramRect.y = 480;
    gVramRect.w = 128;
    gVramRect.h = 32;
    MoveImage(&gVramRect, 640, 256);
    DrawSync(0);

    gVramRect.x = 448;
    gVramRect.y = 480;
    gVramRect.w = 72;
    gVramRect.h = 32;
    MoveImage(&gVramRect, 640, 288);
    DrawSync(0);

    {
        void *timBuffer = AllocHeapBlock(0x1e014);
        request = RequestSitePartRead(timParts[0][gCurrentDisc], timBuffer);
        while (PollCdRequest(request) == 0) {
        }
        UploadTimBuffer(320, 256);
    }
    {
        void *timBuffer = AllocHeapBlock(0x154);
        request = RequestSitePartRead(timParts[1][gCurrentDisc], timBuffer);
        while (PollCdRequest(request) == 0) {
        }
        UploadTimBuffer(576, 256);
    }
    {
        void *timBuffer = AllocHeapBlock(0x174);
        request = RequestSitePartRead(timParts[2][gCurrentDisc], timBuffer);
        while (PollCdRequest(request) == 0) {
        }
        UploadTimBuffer(596, 256);
    }
    {
        void *timBuffer = AllocHeapBlock(0x20c);
        request = RequestSitePartRead(timParts[3][gCurrentDisc], timBuffer);
        while (PollCdRequest(request) == 0) {
        }
        UploadTimBuffer(618, 256);
    }
    {
        void *timBuffer = AllocHeapBlock(0x2d4);
        request = RequestSitePartRead(timParts[4][gCurrentDisc], timBuffer);
        while (PollCdRequest(request) == 0) {
        }
        UploadTimBuffer(576, 264);
    }
    DrawSync(0);
    InitSprite((GsSPRITE *)gTrackImage, 32, 0, 256, 240,
                  GetTPage(2, 1, 320, 256), 0, 0, 0, 0, 0x5a000020, 0, 0);

    gVramRect.x = 576;
    gVramRect.y = 256;
    gVramRect.w = 20;
    gVramRect.h = 8;
    MoveImage(&gVramRect, 443, 412);
    gDecodedAudio = AllocHeapBlock(0x1000);
    SpuSetTransferCallback(SwapSpuIrqAddr);
    gDecodedDataFlag = 5;
    SpuSetIRQCallback(ReadDecodedAudio);
    SpuSetIRQAddr(gSpuIrqAddr = 0x100);
    gVoiceScreenState = VOICE_FIRST_CLIP;
    LoadVoiceScreenClips();
    while (PollVoiceScreenClips() == 0) {
    }

    do {
        BeginFrame();
        if (gVoiceScreenState == VOICE_FIRST_CLIP || gVoiceScreenState == VOICE_LAST_CLIP) {
            if (gHasEndRollSpheres == 1) {
                StepAndSortEndRollSpheres(0);
            }
            DrawClipFrame();
        } else {
            if (gHasEndRollSpheres == 1) {
                StepAndSortEndRollSpheres(3);
            }
            GsSortFastSprite((GsSPRITE *)gTrackImage, &gOt[gActiveBuffer], 350);
            DrawSync(0);
        }
        EndSceneFrame();
        if (gHasEndRollSpheres == 0 && gVoiceScreenState == VOICE_SPEAK_BEFORE) {
            VSync(2);
        }
    } while ((u_char)StepVoiceScreen() != 1);

    if (gHasEndRollSpheres == 1) {
        do {
            BeginFrame();
            StepEndRollSpheresCollapse();
            EndSceneFrame();
        } while (gSphereCollapseDone == 0);
    }
    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(0);
    SpuSetTransferCallback(0);
    FreeHeapBlock(gDecodedAudio);
    SpuFree(gSpeechSpuAddr);

    gVramRect.x = 640;
    gVramRect.y = 256;
    gVramRect.w = 128;
    gVramRect.h = 32;
    MoveImage(&gVramRect, 320, 480);
    DrawSync(0);
    gVramRect.x = 640;
    gVramRect.y = 288;
    gVramRect.w = 72;
    gVramRect.h = 32;
    MoveImage(&gVramRect, 448, 480);
    DrawSync(0);
    LoadAltLoadingImage();
}

u_char gTrackTileSequence[12] = { 0, 0, 0, 2, 2, 1, 1, 0, 0, 0, 0, 0 };
short gStateFrame;
short gTileStep;
short gTrackPeakLevel;

int StepVoiceScreen(void)
{
    u_char tileDest[3][2] = { { 155, 156 }, { 154, 157 }, { 157, 156 } };
    u_char tileSize[3][2] = { { 20, 8 }, { 22, 8 }, { 18, 14 } };
    u_char tileX[3] = { 0, 20, 42 };
    SpuVoiceAttr voice;
    RECT *rect;

    switch (gVoiceScreenState) {
    case VOICE_FIRST_CLIP:
        if (PlayFirstVoiceScreenClip() == 0) {
            if (gTrackIndex < 639) {
                gVoiceScreenState++;
            } else {
                gVoiceScreenState += 2;
            }
            gTileStep = 0;
            gStateFrame = 0;
        }
        gClipFrameInfo.x = 150;
        gClipFrameInfo.y = 300;
        break;
    case VOICE_SPEAK_BEFORE:
        rect = &gVramRect;
        gVramRect.x = 576;
        gVramRect.y = 264;
        gVramRect.w = 22;
        gVramRect.h = 16;
        MoveImage(rect, 442, 411);
        {
            int index = gTrackTileSequence[0];
            int dx = tileX[index];
            gVramRect.y = 256;
            gVramRect.x = 576 + dx;
            gVramRect.w = tileSize[index][0];
            gVramRect.h = tileSize[index][1];
            MoveImage(rect, 288 + tileDest[index][0], 256 | tileDest[index][1]);
        }
        DrawSync(0);
        if (gStateFrame == 0) {
            SpuVoiceAttr *attr;
            gVramRect.w = 20;
            gVramRect.x = 576;
            gVramRect.y = 256;
            gVramRect.h = 8;
            MoveImage(rect, 443, 412);
            DrawSync(0);
            attr = &voice;
            attr->mask = SPU_VOICE_PITCH;
            voice.voice = SPU_23CH;
            voice.pitch = 0x85f;
            SpuSetKeyOnWithAttr(attr);
        }
        if (gStateFrame < gSpeechFrameCount) {
            if (gTileStep < 6 && gSpeechClipCount != 0) {
                gTileStep++;
            } else {
                gTileStep = 0;
                if (gSpeechClipCount != 0) {
                    gSpeechClipCount--;
                }
            }
            {
                u_char *cell;
                u_char tile;
                cell = &gSpeechTileSequence[gTileStep];
                {
                    int dx = tileX[*cell];
                    gVramRect.x = 576 + dx;
                    gVramRect.y = 256;
                }
                gVramRect.w = tileSize[*cell][0];
                gVramRect.h = tileSize[*cell][1];
                tile = *cell;
                MoveImage(&gVramRect, 288 + tileDest[tile][0], 256 | tileDest[tile][1]);
            }
            gStateFrame++;
        } else {
            SpuSetKey(SPU_OFF, SPU_23CH);
            SpuFree(gSpeechSpuAddr);
            gStateFrame = 0;
            gVoiceScreenState++;
        }
        break;
    case VOICE_PLAY_TRACK:
        if (gStateFrame == 0) {
            RECT *rect = &gVramRect;
            gVramRect.x = 576;
            gVramRect.y = 256;
            gVramRect.w = 20;
            gVramRect.h = 8;
            MoveImage(rect, 443, 412);
            {
                int index = gTrackTileSequence[0];
                int dx = tileX[index];
                gVramRect.y = 256;
                gVramRect.x = 576 + dx;
                gVramRect.w = tileSize[index][0];
                gVramRect.h = tileSize[index][1];
                MoveImage(rect, 288 + tileDest[index][0], 256 | tileDest[index][1]);
            }
            DrawSync(0);
            gEndRollState = END_ROLL_PENDING;
            gTrackDone = 0;
            gTrackXaFile = gTracks[gTrackIndex].xaFile;
            UnmuteCdAudio();
            if (CdSync(1, 0) == 0) {
                CdControlF(CdlPause, 0);
            }
            SeekXaFile();
            SpuSetIRQ(SPU_ON);
            StartXaTrack(gTracks[gTrackIndex].source);
            gTrackPeakLevel = GetCdPeakLevel();
        }
        if ((short)IsTrackDone() == 0) {
            RECT *rect = &gVramRect;
            gVramRect.x = 576;
            gVramRect.y = 264;
            gVramRect.w = 22;
            gVramRect.h = 16;
            MoveImage(rect, 442, 411);
            DrawSync(0);
            if (gTrackPeakLevel > 0) {
                u_char *cell;
                u_char tile;
                if (gTileStep < 8) {
                    gTileStep++;
                } else {
                    gTileStep = 0;
                    gTrackPeakLevel = GetCdPeakLevel();
                }
                cell = &gTrackTileSequence[gTileStep];
                {
                    int dx = tileX[*cell];
                    gVramRect.x = 576 + dx;
                    gVramRect.y = 256;
                }
                gVramRect.w = tileSize[*cell][0];
                gVramRect.h = tileSize[*cell][1];
                tile = *cell;
                MoveImage(&gVramRect, 288 + tileDest[tile][0], 256 | tileDest[tile][1]);
            } else {
                {
                    int index = gTrackTileSequence[0];
                    int dx = tileX[index];
                    gVramRect.x = 576 + dx;
                    gVramRect.y = 256;
                    gVramRect.w = tileSize[index][0];
                    gVramRect.h = tileSize[index][1];
                    MoveImage(rect, 288 + tileDest[index][0], 256 | tileDest[index][1]);
                }
                DrawSync(0);
                gTrackPeakLevel = GetCdPeakLevel();
            }
            gStateFrame = 1;
        } else {
            if (gHasEndRollSpheres == 0) {
                gVoiceScreenState += 2;
            } else {
                gVoiceScreenState++;
            }
            gStateFrame = 0;
            gTileStep = 0;
            MuteCdAudio();
            SpuSetIRQ(SPU_OFF);
            SpuSetIRQCallback(0);
            SpuSetTransferCallback(0);
        }
        break;
    case VOICE_SPEAK_AFTER:
        rect = &gVramRect;
        gVramRect.x = 576;
        gVramRect.y = 264;
        gVramRect.w = 22;
        gVramRect.h = 16;
        MoveImage(rect, 442, 411);
        {
            int index = gTrackTileSequence[0];
            int dx = tileX[index];
            gVramRect.y = 256;
            gVramRect.x = 576 + dx;
            gVramRect.w = tileSize[index][0];
            gVramRect.h = tileSize[index][1];
            MoveImage(rect, 288 + tileDest[index][0], 256 | tileDest[index][1]);
        }
        DrawSync(0);
        if (gStateFrame == 0) {
            SpuVoiceAttr *attr;
            gVramRect.w = 20;
            gVramRect.x = 576;
            gVramRect.y = 256;
            gVramRect.h = 8;
            MoveImage(rect, 443, 412);
            DrawSync(0);
            attr = &voice;
            attr->mask = SPU_VOICE_PITCH;
            voice.voice = SPU_23CH;
            voice.pitch = 0x85f;
            SpuSetKeyOnWithAttr(attr);
        }
        if (gStateFrame < gSpeechFrameCount) {
            if (gTileStep < 6 && gSpeechClipCount != 0) {
                gTileStep++;
            } else {
                gTileStep = 0;
                if (gSpeechClipCount != 0) {
                    gSpeechClipCount--;
                }
            }
            {
                u_char *cell;
                u_char tile;
                cell = &gSpeechTileSequence[gTileStep];
                {
                    int dx = tileX[*cell];
                    gVramRect.x = 576 + dx;
                    gVramRect.y = 256;
                }
                gVramRect.w = tileSize[*cell][0];
                gVramRect.h = tileSize[*cell][1];
                tile = *cell;
                MoveImage(&gVramRect, 288 + tileDest[tile][0], 256 | tileDest[tile][1]);
            }
            gStateFrame++;
        } else {
            SpuSetKey(SPU_OFF, SPU_23CH);
            SpuFree(gSpeechSpuAddr);
            gStateFrame = 0;
            gVoiceScreenState++;
        }
        break;
    case VOICE_LAST_CLIP:
        if (gStateFrame == 0 && gHasEndRollSpheres == 1) {
            u_char xaFile;
            gTrackIndex = 734;
            xaFile = gTracks[734].xaFile;
            gEndRollState = END_ROLL_PENDING;
            gTrackDone = 0;
            gTileStep = 0;
            gTrackXaFile = xaFile;
            UnmuteCdAudio();
            if (CdSync(1, 0) == 0) {
                CdControlF(CdlPause, 0);
            }
            SeekXaFile();
            StartXaTrack(gTracks[gTrackIndex].source);
            gStateFrame = 1;
        }
        if (PlayLastVoiceScreenClip() == 0) {
            gTileStep = 0;
            gStateFrame = 0;
            gVoiceScreenState++;
            MuteCdAudio();
            if (gHasEndRollSpheres == 0) {
                rect = &gVramRect;
                gVramRect.w = 320;
                gVramRect.x = 0;
                gVramRect.y = 0;
                gVramRect.h = 480;
                ClearImage(rect, 0, 0, 0);
                DrawSync(0);
            }
        }
        gClipFrameInfo.x = 150;
        gClipFrameInfo.y = 300;
        break;
    }
    if (gVoiceScreenState >= VOICE_DONE) {
        gEndRollState = END_ROLL_NONE;
        return 1;
    }
    return 0;
}

void KeyOnVoice(int voice)
{
    SpuVoiceAttr attr;

    attr.mask = SPU_VOICE_PITCH;
    attr.voice = 1 << voice;
    attr.pitch = voice * 735 / 12 + 735;
    SpuSetKeyOnWithAttr(&attr);
}

void KeyOffVoice(int voice)
{
    SpuSetKey(SPU_OFF, 1 << voice);
}
